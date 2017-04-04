

cbuffer externalData	: register(b0)
{
	float3 cameraPosition;
	float3 cameraForward;
	int MaxOctreeDepth;
	int wvWidth;    // world Voxel width Entire space is made up of a (wvWidth + wvWidth)**3 area
}

struct VertexToPixel
{
	float4 position						: SV_POSITION;
	noperspective float3 viewRay		: VRAY;
	float2 uv							: TEXCOORD;
};

struct Node
{
	float3			position;
	float3			normal;
	float3			color;
	int             flagBits;       // 0 empty, 1 pointer to nodes, 2 leaf node
	int             childPointer;	// pointer to child 8 tile chunch of the octree, an offset index
	uint			padding;		// ensures the 128 bit allignment
};

StructuredBuffer<Node> octree : register(t0);
static float maxDist = 100.0f;

/*
SHADERTOY REFERENCE STUFF from my account EKnapik
*/
struct Sphere {
	float radius;
	float3 pos;
};

// Global Object Definitions
static Sphere sphere1;
static Sphere sphere2;


// ----- Intersect Functions -----

// equation of plane dot((p - po), normal) = 0
// this assumes plane at y = 0 and normal is (0.0, 1.0, 0.0)
float iPlane(float3 ro, float3 rayDir) {
	return -ro.y / rayDir.y;
}

// Using a plane limited on the x axis to more closely mimic
// Turner Witter's paper
float iPlaneLimited(float3 ro, float3 rayDir) {
	float result = -ro.y / rayDir.y;
	float3 pos = ro + result*rayDir;
	if (pos.x > 3.0 || pos.x < -5.0) {
		return -1.0;
	}
	return result;
}

float3 nPlane() {
	return float3(0.0, 1.0, 0.0);
}

float iSphere(float3 ro, float3 rayDir, Sphere sphere) {
	// solving the parametric sphere equation
	// solving the quadradic
	float3 oc = ro - sphere.pos;
	float b = dot(oc, rayDir);
	float c = dot(oc, oc) - sphere.radius*sphere.radius;
	float h = b*b - c;
	if (h < 0.0) return -1.0;
	float t = -b - sqrt(h);
	return t;
}

float3 nSphere(float3 pos, Sphere sphere) {
	return (pos - sphere.pos) / sphere.radius;
}



// returns the id of the object hit
// simultaneously sets the time that an object was hit
float intersect(in float3 ro, in float3 rayDir, out float resT) {
	resT = 10000.0; // infinity kinda
	float id = -1.0; // the object hit
	float tsphere1 = iSphere(ro, rayDir, sphere1);
	float tsphere2 = iSphere(ro, rayDir, sphere2);
	float tplane = iPlaneLimited(ro, rayDir);
	if (tsphere1 > 0.0) {
		id = 2.0; // intersected with sphere1
		resT = tsphere1; // setting the time value the sphere is at
	}
	if (tsphere2 > 0.0 && tsphere2 < resT) {
		id = 3.0;
		resT = tsphere2;
	}
	if (tplane > 0.0 && tplane < resT) {
		id = 1.0;
		resT = tplane;
	}

	return id;
}


float3 calColorRec(float3 rayOr, float3 rayDir) {
	float t;
	float id = intersect(rayOr, rayDir, t);

	float3 pos = rayOr + rayDir*(t);
	// need this to prevent shelf shading
	float3 posShadow = rayOr + rayDir*(t - 0.0001);
	float3 nor;
	float3 reflectEye; // rayDir is the eye to position
	float3 lightPos = float3(1.5, 5.0, 6.0);
	float3 lightDir = normalize(lightPos - pos);
	float3 lightCol = float3(1.0, 0.9, 0.7);
	float specCoeff, diffCoeff, ambCoeff;
	float spec, diff, shadow;
	float3 amb;

	// set material of object
	float3 material;
	if (id > 0.5 && id < 1.5) { // hit the plane
		nor = nPlane();
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		float tileSize = 2.0;
		float tile = (floor(tileSize*(pos.x + 50)) + floor(tileSize*(pos.z + 50))) % 2.0;
		if (tile > 0.0) {
			material = float3(0.9, 0.1, 0.1);
		}
		else {
			material = float3(0.9, 0.9, 0.1);
		}
		// float marbleCol = fbm(pos.xz + .5*fbm(pos.xz + .2*fbm(pos.xz)));
		// material = vec3(marbleCol);

	}
	else if (id > 1.5 && id < 2.5) { // hit the sphere1
		nor = nSphere(pos, sphere1);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		material = float3(0.8, 0.1, 0.3);
	}
	else if (id > 2.5 && id < 3.5) { // hit the sphere2
		nor = nSphere(pos, sphere2);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		material = float3(0.2, 0.1, 0.8);
	}
	else { // background
		   // cornflower blue
		return float3(0.39, 0.61, 0.94);
	}

	// calculate lighting
	float3 brdf;
	ambCoeff = 0.1;
	diffCoeff = .9454545454;
	specCoeff = .3545454545;
	// hard shadow method
	float trashTime; // this isn't going to be used right now
	shadow = intersect(posShadow, lightDir, trashTime);
	if (shadow > 0.0) {
		shadow = 0.1;
	}
	else {
		shadow = 1.0;
	}
	amb = ambCoeff*float3(1.0, 1.0, 1.0);
	diff = shadow*diffCoeff*clamp(dot(nor, lightDir), 0.0, 1.0);
	spec = shadow*specCoeff*pow(clamp(dot(reflectEye, lightDir), 0.0, 1.0), 30.0);
	brdf = material*lightCol*(diff + spec);
	brdf += amb;
	return brdf;
}


float3 calColor(float3 rayOr, float3 rayDir) {
	float t;
	float id = intersect(rayOr, rayDir, t);

	float3 pos = rayOr + rayDir*(t);
	// need this to prevent shelf shading
	float3 posShadow = rayOr + rayDir*(t - 0.0001);
	float3 nor;
	float3 reflectEye; // rayDir is the eye to position
	float3 lightPos = float3(1.5, 5.0, 6.0);
	float3 lightDir = normalize(lightPos - pos);
	float3 lightCol = float3(1.0, 0.9, 0.7);
	float specCoeff, diffCoeff, ambCoeff;
	float spec, diff, shadow;
	float3 amb;

	// set material of object
	float3 material;
	if (id > 0.5 && id < 1.5) { // hit the plane
		nor = nPlane();
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		float tileSize = 2.0;
		float tile = (floor(tileSize*(pos.x + 50)) + floor(tileSize*(pos.z + 50))) % 2.0;
		if (tile > 0.0) {
			material = float3(0.9, 0.1, 0.1);
		}
		else {
			material = float3(0.9, 0.9, 0.1);
		}

	}
	else if (id > 1.5 && id < 2.5) { // hit the sphere1
		nor = nSphere(pos, sphere1);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		float3 reflectRay = reflect(rayDir, nor);
		float3 reflectColor = calColorRec(posShadow, reflectRay);
		// material color
		material = lerp(float3(.9, .9, .9), reflectColor, .7);
	}
	else if (id > 2.5 && id < 3.5) { // hit the sphere2
		nor = nSphere(pos, sphere2);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		material = float3(0.2, 0.1, 0.8);
	}
	else { // background
		return float3(0.39, 0.61, 0.94);
	}

	// calculate lighting
	float3 brdf;
	ambCoeff = 0.1;
	diffCoeff = .9454545454;
	specCoeff = .3545454545;
	// hard shadow method
	float trashTime; // this isn't going to be used right now
	shadow = intersect(posShadow, lightDir, trashTime);
	if (shadow > 0.0) {
		shadow = 0.1;
	}
	else {
		shadow = 1.0;
	}
	amb = ambCoeff*float3(1.0, 1.0, 1.0);
	diff = shadow*diffCoeff*clamp(dot(nor, lightDir), 0.0, 1.0);
	spec = shadow*specCoeff*pow(clamp(dot(reflectEye, lightDir), 0.0, 1.0), 30.0);
	brdf = material*lightCol*(diff + spec);
	brdf += amb;
	return brdf;
}


float4 main(VertexToPixel input) : SV_TARGET
{
	sphere1.radius = 0.65;
	sphere1.pos = float3(0.1, 1.0, 2.0);
	sphere2.radius = 0.7;
	sphere2.pos = float3(1.2, 1.4, 1.2);

	// camera or eye (where rays start)
	// Straight ahead view
	float3 rayOrigin = cameraPosition;
	// ray direction into image plane
	float3 rayDir = normalize(input.viewRay);

	//render the scene with ray marching
	float3 col = calColor(rayOrigin, rayDir);
	col = col*1.2;
	return float4(col, 1.0);
}

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
// OCTREE STRUCTURE
// look top down on a LH xz grid and follow the normal counter clockwise 4 quadrant orientation
// INDEX [   0    ,     1    ,    2   ,     3   ,     4    ,      5    ,     6   ,      7   , .....]
// Node Pos [(1,1,1), (-1,1,1), (-1,1,-1), (1,1,-1), (1,-1,1), (-1,-1,1), (-1,-1,-1), (1,-1,-1), .....]
//

float4 GetOctaveIndex(float3 pos)
{
	if (pos.x > 0)
	{
		if (pos.y > 0)
		{
			if (pos.z > 0)
			{
				return float4(0, 1, 1, 1);
			}
			else
			{
				return float4(3, 1, 1, -1);
			}
		}
		else
		{
			if (pos.z > 0)
			{
				return float4(4, 1, -1, 1);
			}
			else
			{
				return float4(7, 1, -1, -1);
			}
		}
	}
	else
	{
		if (pos.y > 0)
		{
			if (pos.z > 0)
			{
				return float4(1, -1, 1, 1);
			}
			else
			{
				return float4(2, -1, 1, -1);
			}
		}
		else
		{
			if (pos.z > 0)
			{
				return float4(5, -1, -1, 1);
			}
			else
			{
				return float4(6, -1, -1, -1);
			}
		}
	}
}


/// raymarches through the octree and places the intersection
/// into the iNode.
float intersect(in float3 rayO, in float3 rayDir, out Node iNode)
{
	float curVoxelWidth = wvWidth;
	float4 octaveIndex;
	int octreeIndex;

	Node currentNode = octree[0];
	float t = 0;
	float3 pos;
	while (t < maxDist && currentNode.flagBits != 2)
	{
		pos = rayO + rayDir*t;
		octaveIndex = GetOctaveIndex(pos);
		octreeIndex = octaveIndex.x;
		currentNode = octree[octreeIndex];
		// Traverse down the octree to the leaf node of the current position
		for (int currLevel = 0; currLevel < MaxOctreeDepth; currLevel++)
		{
			// get to new position by moving then check again
			if (currentNode.flagBits == 0)
			{
				t += wvWidth / pow(2, currLevel + 1);
				break;
			}
			else
			{
				pos -= float3(octaveIndex.yzw) * wvWidth / pow(2, currLevel + 1);
				octaveIndex = GetOctaveIndex(pos);
				octreeIndex = octree[octreeIndex].childPointer + octaveIndex.x;
			}
			currentNode = octree[octreeIndex];
		}
	}
	iNode = currentNode;
	return t;
}


float4 main(VertexToPixel input) : SV_TARGET
{
	// Ray Trace for each pixel to intersect SVO
	float3 rayOrigin = cameraPosition;
	float3 rayDir = normalize(input.viewRay);

	// firstRay
	Node hitNode;
	float t = intersect(rayOrigin, rayDir, hitNode);
	if (t >= maxDist)
		return float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 pos = rayOrigin + rayDir * t;
	// need this to prevent shelf shading
	float3 posShadow = rayOrigin + rayDir * (t - 0.0001);
	float3 nor;
	float3 reflectEye; // rayDir is the eye to position

	// LIGHT
	float3 lightPos = float3(1.5, 5.0, -6.0);
	float3 lightDir = normalize(lightPos - pos);
	float3 lightCol = float3(1.0, 0.9, 0.7);
	float specCoeff, diffCoeff, ambCoeff;
	float spec, diff;
	float3 amb;

	// set material of object
	float3 material = hitNode.color;
	nor = hitNode.normal;
	reflectEye = reflect(normalize(rayDir), nor);
	
	// calculate lighting FOR EACH LIGHT
	ambCoeff = 0.1;
	diffCoeff = .9454545454;
	specCoeff = .3545454545;
	// hard shadow method
	float shadow = intersect(posShadow, lightDir, hitNode);
	if (shadow < maxDist) { // did this hit anything?
		shadow = 0.1;
	}
	else {
		shadow = 1.0;
	}

	// Simple Phong Lighting
	float3 finalColor = float3(0.0f, 0.0f, 0.0f);
	amb = ambCoeff*float3(1.0, 1.0, 1.0);
	diff = shadow*diffCoeff*clamp(dot(nor, lightDir), 0.0, 1.0);
	spec = shadow*specCoeff*pow(clamp(dot(reflectEye, lightDir), 0.0, 1.0), 30.0);
	finalColor = material*lightCol*(diff + spec);
	finalColor += amb;

	return float4(finalColor, 1.0f);
}



/*
SHADERTOY REFERENCE STUFF from my account EKnapik
*/
struct Sphere {
	float radius;
	vec3 pos;
};

// Global Object Definitions
Sphere sphere1 = Sphere(0.65, vec3(0.1, 1.0, -1.0));
Sphere sphere2 = Sphere(0.7, vec3(1.2, 1.4, 0.2));

// ----- Intersect Functions -----

// equation of plane dot((p - po), normal) = 0
// this assumes plane at y = 0 and normal is (0.0, 1.0, 0.0)
float iPlane(vec3 ro, vec3 rayDir) {
	return -ro.y / rayDir.y;
}

// Using a plane limited on the x axis to more closely mimic
// Turner Witter's paper
float iPlaneLimited(vec3 ro, vec3 rayDir) {
	float result = -ro.y / rayDir.y;
	vec3 pos = ro + result*rayDir;
	if (pos.x > 3.0 || pos.x < -5.0) {
		return -1.0;
	}
	return result;
}

vec3 nPlane() {
	return vec3(0.0, 1.0, 0.0);
}

float iSphere(vec3 ro, vec3 rayDir, Sphere sphere) {
	// solving the parametric sphere equation
	// solving the quadradic
	vec3 oc = ro - sphere.pos;
	float b = dot(oc, rayDir);
	float c = dot(oc, oc) - sphere.radius*sphere.radius;
	float h = b*b - c;
	if (h < 0.0) return -1.0;
	float t = -b - sqrt(h);
	return t;
}

vec3 nSphere(vec3 pos, Sphere sphere) {
	return (pos - sphere.pos) / sphere.radius;
}

float fbm(vec2 p) {

	float ql = length(p);
	//p.x += 0.05*sin(0.21*iGlobalTime+ql*2.0);
	//p.y += 0.05*sin(0.53*iGlobalTime+ql*4.0);
	// My TEST
	float total = 0.0;
	float freq = 0.005250;
	float lacunarity = 2.8;
	float gain = 0.6;
	float amp = gain;

	for (int i = 0; i < 5; i++) {
		total += texture(iChannel0, p*freq).r*amp;
		freq *= lacunarity;
		amp *= gain;
	}

	return total;
}



// returns the id of the object hit
// simultaneously sets the time that an object was hit
float intersect(in vec3 ro, in vec3 rayDir, out float resT) {
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


vec3 calColorRec(vec3 rayOr, vec3 rayDir) {
	float t;
	float id = intersect(rayOr, rayDir, t);

	vec3 pos = rayOr + rayDir*(t);
	// need this to prevent shelf shading
	vec3 posShadow = rayOr + rayDir*(t - 0.0001);
	vec3 nor;
	vec3 reflectEye; // rayDir is the eye to position
	float time = 0.5*iGlobalTime;
	//vec3 lightPos = vec3(3.0*sin(time), 3.0, 3.0*cos(time));
	vec3 lightPos = vec3(1.5, 5.0, 6.0);
	vec3 lightDir = normalize(lightPos - pos);
	vec3 lightCol = vec3(1.0, 0.9, 0.7);
	float specCoeff, diffCoeff, ambCoeff;
	float spec, diff, shadow;
	vec3 amb;

	// set material of object
	vec3 material;
	if (id > 0.5 && id < 1.5) { // hit the plane
		nor = nPlane();
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		float tileSize = 2.0;
		float tile = mod(floor(tileSize*pos.x) + floor(tileSize*pos.z), 2.0);
		if (tile > 0.0) {
			material = vec3(0.9, 0.1, 0.1);
		}
		else {
			material = vec3(0.9, 0.9, 0.1);
		}
		// float marbleCol = fbm(pos.xz + .5*fbm(pos.xz + .2*fbm(pos.xz)));
		// material = vec3(marbleCol);

	}
	else if (id > 1.5 && id < 2.5) { // hit the sphere1
		nor = nSphere(pos, sphere1);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		material = vec3(0.8, 0.1, 0.3);
	}
	else if (id > 2.5 && id < 3.5) { // hit the sphere2
		nor = nSphere(pos, sphere2);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		material = vec3(0.2, 0.1, 0.8);
	}
	else { // background
		   // cornflower blue
		return vec3(0.39, 0.61, 0.94);
	}

	// calculate lighting
	vec3 brdf;
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
	amb = ambCoeff*vec3(1.0, 1.0, 1.0);
	diff = shadow*diffCoeff*clamp(dot(nor, lightDir), 0.0, 1.0);
	spec = shadow*specCoeff*pow(clamp(dot(reflectEye, lightDir), 0.0, 1.0), 30.0);
	brdf = material*lightCol*(diff + spec);
	brdf += amb;
	return brdf;
}


vec3 calColor(vec3 rayOr, vec3 rayDir) {
	float t;
	float id = intersect(rayOr, rayDir, t);

	vec3 pos = rayOr + rayDir*(t);
	// need this to prevent shelf shading
	vec3 posShadow = rayOr + rayDir*(t - 0.0001);
	vec3 nor;
	vec3 reflectEye; // rayDir is the eye to position
	float time = 0.5*iGlobalTime;
	//vec3 lightPos = vec3(3.0*sin(time), 3.0, 3.0*cos(time));
	vec3 lightPos = vec3(1.5, 5.0, 6.0);
	vec3 lightDir = normalize(lightPos - pos);
	vec3 lightCol = vec3(1.0, 0.9, 0.7);
	float specCoeff, diffCoeff, ambCoeff;
	float spec, diff, shadow;
	vec3 amb;

	// set material of object
	vec3 material;
	if (id > 0.5 && id < 1.5) { // hit the plane
		nor = nPlane();
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		float tileSize = 2.0;
		float tile = mod(floor(tileSize*pos.x) + floor(tileSize*pos.z), 2.0);
		if (tile > 0.0) {
			material = vec3(0.9, 0.1, 0.1);
		}
		else {
			material = vec3(0.9, 0.9, 0.1);
		}
		// float marbleCol = fbm(pos.xz + .5*fbm(pos.xz + .2*fbm(pos.xz)));
		// material = vec3(marbleCol);

	}
	else if (id > 1.5 && id < 2.5) { // hit the sphere1
		nor = nSphere(pos, sphere1);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		vec3 reflectRay = reflect(rayDir, nor);
		vec3 reflectColor = calColorRec(posShadow, reflectRay);
		// material color
		material = mix(vec3(.9), reflectColor, .7);
	}
	else if (id > 2.5 && id < 3.5) { // hit the sphere2
		nor = nSphere(pos, sphere2);
		reflectEye = reflect(normalize(rayDir), nor);
		// material color
		material = vec3(0.2, 0.1, 0.8);
	}
	else { // background
		return vec3(0.39, 0.61, 0.94);
	}

	// calculate lighting
	vec3 brdf;
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
	amb = ambCoeff*vec3(1.0, 1.0, 1.0);
	diff = shadow*diffCoeff*clamp(dot(nor, lightDir), 0.0, 1.0);
	spec = shadow*specCoeff*pow(clamp(dot(reflectEye, lightDir), 0.0, 1.0), 30.0);
	brdf = material*lightCol*(diff + spec);
	brdf += amb;
	return brdf;
}


// CAMERA SETTING
mat3 mkCamMat(in vec3 rayOrigin, in vec3 lookAtPoint, float roll) {
	vec3 cw = normalize(lookAtPoint - rayOrigin);
	vec3 cp = vec3(sin(roll), cos(roll), 0.0); //this is a temp right vec for cross determination
	vec3 cu = normalize(cross(cw, cp));
	vec3 cv = normalize(cross(cu, cw));

	return mat3(cu, cv, cw);
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 q = fragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0*q;
	p.x *= iResolution.x / iResolution.y;

	// camera or eye (where rays start)
	// Straight ahead view
	vec3 rayOrigin = vec3(1.0, 1.1, 2.0);
	vec3 lookAtPoint = vec3(1.0, 1.1, -1.0);
	// top down view
	//vec3 rayOrigin = vec3(1.2, 6.4, 0.2);
	//vec3 lookAtPoint = vec3(1.2, -1.0, 0.3);
	float focalLen = 1.5; // how far camera is from image plane
	mat3 camMat = mkCamMat(rayOrigin, lookAtPoint, 0.0);

	// ray direction into image plane
	vec3 rayDir = camMat * normalize(vec3(p.xy, focalLen));

	//render the scene with ray marching
	vec3 col = calColor(rayOrigin, rayDir);
	col = col*1.2;
	fragColor = vec4(col, 1.0);
	//fragColor = vec4(.9); // that off white
}
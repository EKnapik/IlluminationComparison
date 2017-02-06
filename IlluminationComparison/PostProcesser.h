#pragma once
class PostProcesser
{
public:
	PostProcesser();
	~PostProcesser();
};

/*
given the current back buffer and has a pointer to the gbuffer
places contents back to the back buffer when done
Not as efficient but easier to manage when thinking of infinite memory.
*/
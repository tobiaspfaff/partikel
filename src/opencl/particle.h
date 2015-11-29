
struct Domain {
	float2 offset;
	uint2 size;
	float dx;
	float pad;
};

inline int2 getGridPos(float px, float py, struct Domain domain)
{
	float x = (px - domain.offset.x) / domain.dx;
	float y = (py - domain.offset.y) / domain.dx;
	return (int2)((int)x, (int)y);
}

inline uint getGridHash(int2 pos, uint2 gridSize)
{
	// wrap addressing
	return ((uint)pos.x & (gridSize.x - 1)) +
		   ((uint)pos.y & (gridSize.y - 1)) * gridSize.x;
}

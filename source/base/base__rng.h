
//~NOTE(tbt): 'noise' via hashing a position

Function unsigned int Noise1U (unsigned int a);
Function int          Noise2I (V2I a);
Function float        Noise2F (V2F a);

//~NOTE(tbt): pseudo-random sequence

Function void RandIntInit    (int seed);
Function int  RandIntNextRaw (void);
Function int  RandIntNext    (int min, int max);

//~NOTE(tbt): perlin noise

Function float Perlin2D (V2F a, float freq, int depth);

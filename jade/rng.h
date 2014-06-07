#ifndef RNG_H
#define RNG_H

namespace jade
{
// Random Number Declarations
class RNG {
public:
    RNG(unsigned int seed = 5489UL) {
        mti = N+1; /* mti==N+1 means mt[N] is not initialized */
        Seed(seed);
    }
	
    void Seed(unsigned int seed) const;
    float RandomFloat() const;
    unsigned int RandomUInt() const;
	
private:
    static const int N = 624;
    mutable unsigned int mt[N]; /* the array for the state vector  */
    mutable int mti;
};
	
}

#endif

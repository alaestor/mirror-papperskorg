#ifndef FGLMATHS_H_INCLUDED
#define FGLMATHS_H_INCLUDED

/// Arcane Magic
// powered by GCC
// sponsored by Abercrombie and Fitch (w/ snapbacks)

static inline uint32_t fmaths_roundup_power2_next(uint32_t const Value) // round up to the next power of 2
{
    uint32_t ResultMask; // set up for for the ritual

///-----------WARNING-----------///
///---FORBIDDEN-ARCANE-MAGIC---///
    __asm__("bsrl %0, %1\n\t"
        : "=g"(ResultMask)
        : "g" (Value)
        : "1"); // does magic ritual
///-----------WARNING-----------///
///---FORBIDDEN-ARCANE-MAGIC---///

    register uint32_t const TrueValue = 1 << ResultMask; // alters magic with evil chant

    ResultMask = (TrueValue - 1) & Value; // summon the demon

    return (ResultMask) ? TrueValue << 1 : TrueValue; // return the demon to the realm from which it came
}

static inline uint32_t fmaths_roundup_power2_ceiling(uint32_t const Value, uint32_t const Ceiling) // round up to the ceiling power of 2
{
    uint32_t const Mask = Ceiling - 1;
    uint32_t RoundResult = Mask & Value;
    uint32_t TrueValue = Value & (~Mask);

    return RoundResult ? TrueValue + Ceiling : TrueValue;
}


#endif // FGLMATHS_H_INCLUDED

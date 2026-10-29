#include <cstddef>
#include <cstdint>
#include <cstdio>

// "static", "consteval", and "constexpr" are used unexplained here;
// in this case, because their usage disallows undefined behavior from occuring,
// meaning that these statements are as provably true as possible.

// the poor man's assertion function, for reasons
consteval void exassert(bool result)
{ if (result == false) throw; }

consteval bool pointer_proofs()
{
    std::uint16_t i{ 1337 };
    // "initialize 16bit unsigned integer i with the integer value 1337"

    exassert(i == 1337);
    // "the value of i is 1337"

    exassert(sizeof(i) == sizeof(std::uint16_t));
    // "the size of i is equal to the size of a 16bit unsigned integer"

    exassert(sizeof(i) == 2); // bytes
    // "the size of i is equal to integer two"

    std::uint16_t const *const p{ &i };
    // "initialize 16bit unsigned integer pointer p with the address of i"

    // Note: we would say now that `p` is "a reference to", or "aliases",
    // or "points to" `i` because its value is the address of `i`...
    // But calling C pointers (like `int*`) a "reference" is oldschool terminology,
    // and must not to be confused with C++'s native reference types
    // e.g. "int reference" `int&` - this & is NOT address-of

    // address length varies by architecture; platform-dependent
    // but all pointers are the same size, as their values are an address
    exassert(sizeof(p) == sizeof(std::byte*));
    exassert(sizeof(p) == sizeof(void*));
    exassert(sizeof(p) == sizeof(int*));
    exassert(sizeof(p) == sizeof(double*));
    exassert(sizeof(p) == sizeof(&pointer_proofs));
    // sizeof(p) == 4 on 32bit architectures, 8 on 64bit

    // `p` is stored at an address distinct from `i`
    exassert((void*)&p != (void*)&i);
    // "address of p is not equal to the address of i"

    // `p`'s value is the address of `i`
    exassert((void*)p == (void*)&i);
    // "the value of p is equal to the address of i"

    // 'dereferencing' `p` resolves to the value stored at i
    exassert(*p == i);
    // "dereferencing p results in a value equal to the value of i"
    // i.e. "p resolves to a value equal to the value of i"

    exassert(*p == 1337);
    // "p resolves to a value equal to 1337"

    return true;
}

// statically assert that pointer_proofs() returns a non-false value.
// this forces it to be evaluated at compile-time where UB is not allowed.
static_assert(pointer_proofs());

consteval bool pointer_array_proofs()
{
    std::uint16_t values[4]{ 10, 20, 30, 40 };
    // "initialize a 16bit unsigned integer array of length 4 with
    // integer values: 10, 20, 30, and 40"

    // the size of uint16_t is two bytes (16 bits)
    exassert(sizeof(std::uint16_t) == 2);

    // the length of the array is equal to four times the size of uint16_t
    // i.e. the array contains space equal to four two-byte objects (8 bytes)
    exassert(sizeof(values) == 4 * sizeof(std::uint16_t));
    exassert(sizeof(values) == 8);
    // Note: this `sizeof` behavior, getting array length, is only possible
    // because the compiler can see that we declared it with `[4]` in scope;
    // empty [] would also be deduced as 4, but passing the address of an array
    // as a raw pointer to another function doesn't let that function magically 
    // know the length of the array; its usually passed as a separate argument.

    // type-coercion: converting a raw C array to a pointer yields
    // the address of its zeroth element
    exassert(values == &values[0]);
    exassert((void*)&values == (void*)&values[0]);

    // the zeroth element holds the value 10
    exassert(*values == 10);
    exassert(values[0] == 10);

    // initialize p with the address of the zeroth element
    std::uint16_t* p = values;
    
    // just a regular ol' pointer to a 16bit unsigned integer; as declared
    exassert(sizeof(*p) == sizeof(std::uint16_t));
    exassert(sizeof(*p) == 2);
    exassert(p == &values[0]);
    exassert(*p == 10);

    // Array indexing is defined in terms of pointer arithmetic.

    // checking value == value
    exassert(values[0] == *(p + 0));
    exassert(values[1] == *(p + 1));
    exassert(values[2] == *(p + 2));

    // checking address == address
    exassert(&values[0] == p + 0);
    exassert(&values[1] == p + 1);
    exassert(&values[2] == p + 2);

    // Pointer arithmetic counts uint16_t OBJECTS.
    std::uint16_t* p1 = p + 1;
    exassert(p1 == &values[1]);
    // first element address minus zeroth element address equals offset of 1
    exassert(p1 - p == 1);

    std::uint16_t* p2 = p + 2;
    exassert(p2 == &values[2]);
    exassert(p2 - p == 2);

    // Each step therefore corresponds to sizeof(uint16_t) == 2 bytes:
    //
    //      p + 0    byte offset 0
    //      p + 1    byte offset 2
    //      p + 2    byte offset 4
    //      p + 3    byte offset 6
    // Live runtime demo in main

    // raw array[index] is just 'syntactic sugar' for arithmetic
    exassert( &(1[p]) == &(values[1]) );
    
    // resolves to same values
    exassert( 1[p] == p[1] );
    exassert( *(1 + p) == *(p + 1) );

    // at the same addresses
    exassert( &(1[p]) == &(p[1]) );
    exassert( &(*(p + 1)) == &(*(1 + p)) );

    // semmantically identical operands; see compiler warnings
    exassert(*(p+1) == p[1]);
    exassert(*(1+p) == 1[p]);

    // Note: this doesn't hold true for class type `operator[]()` members;
    // e.g. `std::array<uint16_t>::operator[](std::size_t index)`
    // i.e. `myStdArray[5]` is modeled as a function, not raw pointer
    // ...tho the assembler's optimizer may boil it down to it in practice.

    return true;
}

static_assert(pointer_array_proofs());


int main()
{
    // demo with printing runtime addresses
    std::uint16_t values[] = {10, 20, 30, 40};
    std::uint16_t* p = values;

    std::printf("sizeof(pointer)  = %zu bytes\n", sizeof(p));
    std::printf("sizeof(*pointer) = %zu bytes\n", sizeof(*p));
    std::printf("sizeof(array)    = %zu bytes\n\n", sizeof(values));

    std::printf("p     = %p\n", static_cast<void*>(p));

    std::printf(
        "p + 1 = %p  (+%zu bytes)\n",
        static_cast<void*>(p + 1),
        sizeof(*p)
    );

    std::printf(
        "p + 2 = %p  (+%zu bytes)\n",
        static_cast<void*>(p + 2),
        2 * sizeof(*p)
    );

    std::printf(
        "p + 3 = %p  (+%zu bytes)\n",
        static_cast<void*>(p + 3),
        3 * sizeof(*p)
    );

    /*
        p + i advances by i OBJECTS, not i bytes.

        Conceptually:

            address(p + i)
                = address(p) + i * sizeof(*p)

        Here sizeof(*p) == sizeof(uint16_t) == 2, so:

            p       -> 0x...00
            p + 1   -> 0x...02
            p + 2   -> 0x...04
            p + 3   -> 0x...06

        sizeof(p) is irrelevant to that stride.
        It describes the pointer itself, not the memory it aliases.
    */

    return 0;
}

/// Example output on 64bit gcc 16.2 @ -Wall -Wextra -pedantic -std=c++26 -O3
/*** Compiler stderr

 <source>: In function 'consteval bool pointer_array_proofs()':
 <source>:138:20: warning: self-comparison always evaluates to true [-Wtautological-compare]
 138 |     exassert( 1[p] == p[1] );
 |               ~~~~ ^~ ~~~~
 <source>:139:24: warning: self-comparison always evaluates to true [-Wtautological-compare]
 139 |     exassert( *(1 + p) == *(p + 1) );
 |               ~~~~~~~~ ^~ ~~~~~~~~
 <source>:142:23: warning: self-comparison always evaluates to true [-Wtautological-compare]
 142 |     exassert( &(1[p]) == &(p[1]) );
 |               ~~~~~~~ ^~ ~~~~~~~
 <source>:143:27: warning: self-comparison always evaluates to true [-Wtautological-compare]
 143 |     exassert( &(*(p + 1)) == &(*(1 + p)) );
 |               ~~~~~~~~~~~ ^~ ~~~~~~~~~~~
 <source>:146:21: warning: self-comparison always evaluates to true [-Wtautological-compare]
 146 |     exassert(*(p+1) == p[1]);
 |              ~~~~~~ ^~ ~~~~
 <source>:147:21: warning: self-comparison always evaluates to true [-Wtautological-compare]
 147 |     exassert(*(1+p) == 1[p]);
 |              ~~~~~~ ^~ ~~~~
*/
/*** Program stdout

 sizeof(pointer)  = 8 bytes
 sizeof(*pointer) = 2 bytes
 sizeof(array)    = 8 bytes

 p     = 0x7ffc8ff91808
 p + 1 = 0x7ffc8ff9180a  (+2 bytes)
 p + 2 = 0x7ffc8ff9180c  (+4 bytes)
 p + 3 = 0x7ffc8ff9180e  (+6 bytes)

*/

How to add a Stealth Data area in your application
--------------------------------------------------

Adding Stealth in your Data section is easier than in the code section.

1) Include "StealthDataArea.h" in one of your source code files.

2) In the global variables section, just call the following macro with the required Stealth Area size:

#define STEALTH_AREA_SIZE 0x10000    // <-- Change this to adjust the size of the Stealth Area

STEALTH_DATA_AREA(STEALTH_AREA_SIZE);

3) Now, you have defined a block of global memory in your compiler but in order to avoid
   that compiler optimizations won't remove that allocation, you have to make a reference somewhere
   in your code the Stealth Data Area. Just insert the "REFERENCE_STEALTH_DATA_AREA" and make sure that it's never executed.
   Example:

---
int main(
    int argc, 
    const char * argv[])
{
    int DummyStealthVar = 0x11111111;

    // Put a dummy reference, so the compiler won't skip the generation of the 
    // Stealth data area for not being referenced. Notice this conditional 
    // is never executed
    if (DummyStealthVar == 0x11223344)
    {
        REFERENCE_STEALTH_DATA_AREA;
    }
    
    // Your code here

    return 0;
}

----

Please, check the example file "Example_StealthInDATAsection.c"


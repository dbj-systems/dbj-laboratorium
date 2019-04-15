#include "dbjclib.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

// http://www.ciphersbyritter.com/NEWS4/RANDC.HTM
typedef unsigned long UL;
#define znew   (z=36969*(z&65535)+(z>>16))
#define wnew   (w=18000*(w&65535)+(w>>16))
#define MWC    ((znew<<16)+wnew )
#define SHR3  (jsr^=(jsr<<17), jsr^=(jsr>>13), jsr^=(jsr<<5))
#define CONG  (jcong=69069*jcong+1234567)
#define FIB   ((b=a+b),(a=b-a))
#define KISS  ((MWC^CONG)+SHR3)
#define LFIB4 (c++,t[c]=t[c]+t[UC(c+58)]+t[UC(c+119)]+t[UC(c+178)])
#define SWB   (c++,bro=(x<y),t[c]=(x=t[UC(c+34)])-(y=t[UC(c+19)]+bro))
#define UNI   (KISS*2.328306e-10)
#define VNI   ((long) KISS)*4.656613e-10
#define UC    (unsigned char)  /*a cast operation*/

/*  Global static variables: */
static UL z = 362436069, w = 521288629, jsr = 123456789, jcong = 380116160;
static UL a = 224466889, b = 7584631, t[256];
/* Use random seeds to reset z,w,jsr,jcong,a,b, and the table t[256]*/

static UL x = 0, y = 0, bro; static unsigned char c = 0;

/* Example procedure to set the table, using KISS: */
static void settable(UL i1, UL i2, UL i3, UL i4, UL i5, UL i6)
{
	int i; z = i1; w = i2, jsr = i3; jcong = i4; a = i5; b = i6;
	for (i = 0; i < 256; i = i + 1)  t[i] = KISS;
}

#pragma region tests
/* This is a test main program.  It should compile and print 7  0's. */
static void comprehensive_test(void) {
	int i; UL k;
	settable(12345, 65435, 34221, 12345, 9983651, 95746118);

	for (i = 1; i < 1000001; i++) { k = LFIB4; } printf("%lu\n", k - 1064612766U);
	for (i = 1; i < 1000001; i++) { k = SWB; } printf("%lu\n", k - 627749721U);
	for (i = 1; i < 1000001; i++) { k = KISS; } printf("%lu\n", k - 1372460312U);
	for (i = 1; i < 1000001; i++) { k = CONG; } printf("%lu\n", k - 1529210297U);
	for (i = 1; i < 1000001; i++) { k = SHR3; } printf("%lu\n", k - 2642725982U);
	for (i = 1; i < 1000001; i++) { k = MWC; } printf("%lu\n", k - 904977562U);
	for (i = 1; i < 1000001; i++) { k = FIB; } printf("%lu\n", k - 3519793928U);

}

static void mesured_test()
{
	unsigned long i, xx = 0, seed = time(0);
	long spent;

	settable(seed, 2 * seed, 3 * seed, 4 * seed, 5 * seed, 6 * seed);

	spent = clock();
	for (i = 0; i < 77777777; i++) xx += (KISS + SWB);
	printf("%lu \t", spent = clock() - spent);

	printf("\n");
}

static void simple_test() {

	int i;
	UL k, top = 0xF;

	settable(12345, 65435, 34221, 12345, 9983651, 95746118);

	printf("\n-----------------------------------------\n");
	for (i = 1; i < top; i++) { k = LFIB4; printf("%lu\t", k); }
	printf("\n-----------------------------------------\n");
	for (i = 1; i < top; i++) { k = SWB; printf("%lu\t", k); }
	printf("\n-----------------------------------------\n");
	for (i = 1; i < top; i++) { k = KISS; printf("%lu\t", k); }
	printf("\n-----------------------------------------\n");
	for (i = 1; i < top; i++) { k = CONG; printf("%lu\t", k); }
	printf("\n-----------------------------------------\n");
	for (i = 1; i < top; i++) { k = SHR3; printf("%lu\t", k); }
	printf("\n-----------------------------------------\n");
	for (i = 1; i < top; i++) { k = MWC; printf("%lu\t", k); }
	printf("\n-----------------------------------------\n");
	for (i = 1; i < top; i++) { k = FIB; printf("%lu\t", k); }
}
#pragma endregion

/*-----------------------------------------------------
   Write your own calling program and try one or more of
   the above, singly or in combination, when you run a
   simulation. You may want to change the simple 1-letter
   names, to avoid conflict with your own choices.

   The last half of the bits of CONG are too regular,
   and it fails tests for which those bits play a
   significant role. CONG+FIB will also have too much
   regularity in trailing bits, as each does. But keep
   in mind that it is a rare application for which
   the trailing bits play a significant role.  CONG
   is one of the most widely used generators of the
   last 30 years, as it was the system generator for
   VAX and was incorporated in several popular
   software packages, all seemingly without complaint.

   Finally, because many simulations call for uniform
   random variables in 0<x<1 or -1<x<1, I use #define
   statements that permit inclusion of such variates
   directly in expressions:  using UNI will provide a
   uniform random real (float) in (0,1), while VNI will
   provide one in (-1,1).

   All of these: MWC, SHR3, CONG, KISS, LFIB4, SWB, FIB
   UNI and VNI, permit direct insertion of the desired
   random quantity into an expression, avoiding the
   time and space costs of a function call. I call
   these in-line-define functions.  To use them, static
   variables z,w,jsr,jcong,a and b should be assigned
   seed values other than their initial values.  If
   LFIB4 or SWB are used, the static table t[256] must
   be initialized.

   A note on timing:  It is difficult to provide exact
   time costs for inclusion of one of these in-line-
   define functions in an expression.  Times may differ
   widely for different compilers, as the C operations
   may be deeply nested and tricky. I suggest these
   rough comparisons, based on averaging ten runs of a
   routine that is essentially a long loop:
   for(i=1;i<10000000;i++) L=KISS; then with KISS
   replaced with SHR3, CONG,... or KISS+SWB, etc. The
   times on my home PC, a Pentium 300MHz, in nanoseconds:
   FIB 49;LFIB4 77;SWB 80;CONG 80;SHR3 84;MWC 93;KISS 157;
   VNI 417;UNI 450;
 */
const unsigned long dbj_random_kiss () {
	return KISS;
}

#ifdef __clang
#pragma clang diagnostic pop
#endif

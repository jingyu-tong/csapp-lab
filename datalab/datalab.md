# 位操作，数据表示
这次lab需要解答一些跟整数以及浮点数相关的一些谜题，每题对操作次数以及可用的符号都进行了限制。而且不允许用超过0-255范围内的常数。
## 1.bitXor
```c
//1
/*
 * bitxor - x^y using only ~ and &
 *   example: bitxor(4, 5) = 1
 *   legal ops: ~ &
 *   max ops: 14
 *   rating: 1
 */
int bitxor(int x, int y) {
  return ~((~(~x & y)) & (~(x & ~y)));
}
```
这题要求我们只用&和~操作实现^，首先我们可以得到`a ^ b = (~a & b) | (a & ~b)`，但是题目不允许我们使用|，因此，我们通过两次取反，可以将|变为&，完成解答。
## 2.tmin
```c
/*
 * tmin - return minimum two's complement integer
 *   legal ops: ! ~ & ^ | + << >>
 *   max ops: 4
 *   rating: 1
 */
int tmin(void) {

  return (0x1 << 31);

}
```
返回最小数的补码，最小数的补码为0x80000000，把1移动到最头上即可。
## 3.isTmax
```c
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {

  return (!(x+1+x+1))&(!!(x+1)); //x is 0 or ox7fff ffff & x != -1
}
```
如果x为最大值，(x + 1) + x + 1 = 0, 满足这个的只有x还为-1时，即x = 0xffffffff，再把这个排除即可。
## 4. allOddBits
```c
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /* options 15 超出范围
  return !( ( (x & 0xaa) ^ 0xaa)
      | ( ( (x >> 8) & 0xaa) ^ 0xaa)
      | ( ( (x >> 16) & 0xaa) ^ 0xaa)
    | ( ( (x >> 24) & 0xaa) ^ 0xaa));
  */
  int y = x >> 16;
  x = x & y;
  y = x >> 8;
  x = x & y;
  return !( (x & 0xaa) ^ 0xaa);
}
```
这题要求判断是否奇数位数都为1。我首先想的是每8位和0xaa求&，然后再异或判断，通过移动位数来求解32位。但是，这个方法要15次操作才能完成，超出了题目的限制。

但是基本思路应该从这出发，但是我们可以做一下精简，让x和x >> 16求&，这样我们可以直接得到32位上每个奇数位的情况，再减半一次，就可以直接用之前提及的方法判断了。
## 5.negate
```c
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x + 1);
}
```
这题要求出输入的相反数，在计算机中以补码的形式存在。假设某数x，那么我们有x + x(反) = 0xffffffff，所以x + x(反) + 1 = 0，那么 0 - x = x(反) + 1，即x(补) = x(反) + 1。那么我们要求的相反数则为x(补)(补) = x(补)(反) + 1.
## 6.isAsciiDigit
```c
//3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int min = 0x30;
  int max = 0x39;
  return (!((x+(~min+1))>>31)) & (!((max+(~x+1))>>31));
}
```
这题要求我们求出x是否在一个范围，我们可以利用x-0x30>=0以及0x39-x>=0来判断，为了判断符号位，我们可以利用>>补位的性质，当x为有符号数时，>>会填补符号位。
## 7.conditional
```c
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  //0 & y + 0xffffffff & z or 0xffffffff & y + 0 & z
  return ( (!x + ~1 + 1) & y) + ( (~!x+1) & z);
}
```
这题要实现类似条件表达式的效果，对于y，我们需要找到x为0时，为0，x不为0,时，为0xffffffff的表达式。我们令x取反，使得要输出y时，x为0，再减去1，即可得到-1的补码，即0xffffffff。类似思路得到z相关的表达式，不再赘述。
## 8.isLessOrEqual
```c
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  // return !((y + (~x + 1)) >> 31); //overflow
  int sign_x = x >> 31;
  int sign_y = y >> 31;
  int sign = sign_x + sign_y;
  int p = !((y + (~x + 1)) >> 31);

  //符号不同，返回x符号即可，符号相同，返回p。
  return (sign & (sign_x & 1)) | ((~sign) & p);
}
```
这题让比较两个数的大小，第一反应是比较y-x的值是否为正，但是有可能会造成溢出(如比较大的y，减比较小的x)。那么我们需要额外考虑一下两个数符号不同时的判断，判断两个数符号是否相同，只要取出两个数符号相加即可。这样，符号不同sign为0xffffffff，我们直接返回x的符号即可，反之，返回相减的结果。
## 9.logicalNeg
```c
//4
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
  x = x | (x >> 16);
  x = x | (x >> 8);
  x = x | (x >> 4);
  x = x | (x >> 2);
  x = x | (x >> 1);
  return (~x & 1);
}
```
这题要求出是否为0,0的补码为0x00000000，第一个思路是找出输入的数是否含有1，有的话就返回0。我们把x不断拆分，最后结果储存到最后一位，取出即可得到答案。这个方法正好是12个option。。。

这个方法刚好12个option，下面有种更加巧妙的方法：
```c
int logicalNeg(int x) {
  int y = ~x + 1; // -x
  return (~y & ~x) >> 31 & 1; //x, y同为0时，取1
}
```
这个方式考虑x和-x的符号，只有x为0，或者为0x80000000时，x和-x的符号才会相同，但是后者均为1，所以当x和-x符号位均为0时，即是我们判断的依据。
## 9.howManyBits
```c
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  //负数~x，正数x
  x = ((x >> 31) & ~x) | (~!(x >> 31) + 1) & x;

  int y, h16, h8, h4, h2, h1;

  y = x>>16;
  h16 = !!y<<4;//高16位是否有1
  x = x>>h16; //有，至少要16位

  y = x>>8;//剩余的8位是否有
  h8 = !!y<<3;//有，至少还要多8位
  x = x>>h8;

  y = x>>4;
  h4 = !!y<<2;
  x = x>>h4;

  y = x>>2;
  h2 = !!y<<1;
  x = x>>h2;

  y = x>>1;
  h1 = !!y<<0;
  x = x>>h1;

	return h16+h8+h4+h2+h1+1+x;

}
```
这题要求出至少需要表示的位数，也就是寻找最高位的1，我们采取二分搜索，首先判断在16位的哪一半，并更新答案，在判断在剩余16位中哪8位，以此类推，最后更新答案。我们判断时，首先忽略符号位(因为要以最高位的1来搜索)，在结果上要把1加上。
## 10.floatScale2
```c
//float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  unsigned sign = uf & 0x80000000;
  unsigned exp = uf & 0x7f800000;
  unsigned frac = uf & 0x007fffff;

  //exp == 0，非规范
  //exp = 0xff，特殊值
  if(!exp) { //0x00
    frac <<= 1;
  } else if(exp ^ 0x7f800000) {//not 0xff
    exp += 0x00800000; //exp + 1 即可
    if(!(exp ^ 0x7f800000)) { //should be infinity
      frac = 0;
    }
  }

  //是0xff，直接返回
  return sign | exp | frac;
}
```
这题要进行float乘2的步骤，接下来几题都是浮点相关的，在这里顺便复习一些IEEE浮点数标准。
![浮点数](/assets/浮点数.jpg)
**首先，IEEE中，用的如下公式表示浮点数：
$$(-1)^sM2^E$$
M为[1.0, 2.0)的小数，E代表次方数。M - 1对应着frac， E + 127对应着exp。
此外，浮点数还有着非规范值的定义。当exp全为0时，此时E对应着1 - 127。当exp全为1时，frac为0，表示$∞$，frac不为0，表示NAN。**

这题要乘2，对于一般情况，没溢出，不为非规范，只要对exp+1即可(+1后判断是否溢出)，对于非规范，exp此时为0，可以将frac进行移位。
## 11.floatFloat2Int
```c
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  int sign = uf >> 31;
  int exp = ((uf&0x7f800000)>>23)-127;
  int frac = (uf&0x007fffff)|0x00800000; //1 + frac

  if(exp > 30) return 0x80000000;
  if(exp < 0) return 0;

  //根据是否超过23判断左移还是右移
  if(exp > 23) frac <<= exp - 23;
  else frac >>= (23 - exp);

  if(sign) {
    frac = ~frac + 1; //重新变为负的
  }
  return frac;
}
```
这题要把float转换为整数，首先根据exp大小判断是否超界，没有的话，根据exp进行移位。最后，如果是负数的话，需要返回他的补码，取反+1即可。
## 12.floatPower2
```c
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  int exp = x + 127;
  if(exp < 0) return 0;
  if(exp >= 255) return 0x7f800000;
  return exp << 23;
}
```
这题要求$2^x$，也就是$1.0*2^x$，那么后23位frac则为0，我们求出exp置位即可，同时注意是否超界。

# 实验结果
首先检测操作以及操作数是否合法
![datalab合法性检测](/assets/datalab合法性检测.png)
接着，检测分数
![datalab分数](/assets/datalab分数.png)
到此，datalab实验结束。

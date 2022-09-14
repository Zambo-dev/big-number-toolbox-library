/**
 * @file bn_calc.c
 * @brief Calculus functions for big numbers struct
 * @author Zambo-dev
 * @date 22/12/2021
 * @copyright Zambo-dev - 2022 | licensed under MIT
*/


#include "../include/bntl.h"

/**
 *	@brief Sum two bn_t numbers storing the result into number1
 *	@param [in,out] number1 Initialized number
 *	@param [in] number2  Initialized number
*/
void bn_sum(bn_t *number1, bn_t *number2)
{	
	if(number1->num == NULL || number2->num == NULL) return;

	ulong tmp = 0, carry = 0, shift, low, high, *num1, *num2;
	byte inc;
	size_t size1, size2;

	/* Extends result if allocated space isn't enough */
	if(number1->size < number2->size)
		bn_ext(number1, number2->size - number1->size);
	
	size1 = number1->size/sizeof(ulong);
	size2 = number2->size/sizeof(ulong);
	shift = sizeof(ulong)*8/2;

	#if(defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN)
		num1 = number1->num;
		num2 = number2->num;
		inc = 1;
	#elif(defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN)
		num1 = number1->num + size1 - 1;
		num2 = number2->num + size2 - 1;
		inc = -1;
	#else
		#error "Unsupported architecture!"
	#endif

	low = ULONG_MAX >> shift;
	high = ULONG_MAX << shift;

	while(size1 > 0 && (size2 > 0 || carry != 0))
	{
		tmp = (size2 > 0) ? (*num2 & low) : 0;
		tmp += (*num1 & low) + carry;
		carry = (tmp & high) >> shift;

		tmp ^= tmp & high;
		*num1 ^= *num1 & low;
		*num1 |= tmp;		

		tmp = (size2 > 0) ? ((*num2 & high) >> shift) : 0;
		tmp += ((*num1 & high) >> shift) + carry;			

		carry = (tmp & high) >> shift;

		tmp <<= shift;
		*num1 ^= *num1 & high;
		*num1 |= tmp;	

		num1 += inc;
		--size1;
		if(size2 > 0)
		{
			num2 += inc;
			--size2;
		}

		if(size1 == 0 && carry != 0)
		{
			bn_ext(number1, sizeof(ulong));
			size1 += sizeof(ulong);
		}
	}

}


/**
 *	@brief Sub two bn_t numbers storing the result into number1
 *	@param [in,out] number1 Initialized number
 *	@param [in] number2 Initialized number
*/
void bn_sub(bn_t *number1, bn_t *number2)
{	

}


/**
 *	@brief Multiply two bn_t numbers storing the result into number1
 *	@param [in,out] number1 Initialized number
 *	@param [in] number2 Initialized number
*/
void bn_mul(bn_t *number1, bn_t *number2)
{

}


/**
 *	@brief Divide two bn_t numbers storing the result into number1 and the rest into rest
 *	@param [in,out] number1 Initialized number
 *	@param [in] number2 Initialized number
 *	@param [in,out] rest Initialized number
*/
void bn_div(bn_t *number1, bn_t *number2, bn_t *rest)
{

}



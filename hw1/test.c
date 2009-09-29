/* Michael O'Keefe
Virginia Hickox
Name this program - Homework#1 */

#include <C8051_SDCC.h>/* include files. This file is available online*/
#include <stdio.h>

void main(void) /* start main function */
{
	int imax, i; /* declare variables before first executable code */
	unsigned char count;
	unsigned char input;

	Sys_Init(); /* Initialize UART, System clock and crossbar*/
	putchar(' '); /* do this because we tell you to */

	while(1) /* begin infinite loop */
	{
		printf("\r\n enter 1 to count to 18 or 2 to count to 270 \r\n");
		input=getchar(); /* get count value */

		if (input=='1') /* if input is 1, count to 18 */
		{
			count=18;
			imax=18;
		}

		else if (input=='2') /* if input is 2, count to 270 */
		{
			count=270;
			imax=270;
		}

		else imax=0; /* for other inputs, skip counting */

		printf("\n count value is %u ",imax);
		printf("\n\n Decimal hex \n\r");

		count=0;

		for(i=0;i<=imax;i++)
		{
			printf("%u %x \n\r", count, count); /* print number as both decimal and hex decimal and hex */
			count++;
		} /* end for loop */
	} /* end while loop */
} /* end main function */
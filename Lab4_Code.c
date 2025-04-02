#include <studio.h>
#include <limits.h>
/*
    VULNERABLE CODE #1
    BUFFER OVERFLOW
*/
void vulnerable_function_one(char *user_input)
{
    char input[100];
    gets(input);
    //Security Flaw (Passing in string directly to printf potetial format string vulnerability)
    printf(user_input);
    //Solution:
    //Add a place holder which allows sanitized(stripped of harmful content) substitution of the string,
    //this apprach avoids user controlled format strings
    printf("%s", user_input);
}

/*
    VULNERABLE CODE #2
    ILLEGAL POINTER VALUE
*/

void vulnerable_function_two(char* str, char target)
{
    while(*str)
    {
        if(*str == target)
        {
            return str;
        }
        str++;
    }
    //Security Flaw (This return can point to a memory location outside the buffer)
    return str;
    //Solution:
    //Ensure we return null if the pointer points out of bounds
    return (*str == target) ? str : NULL;
}

/*
    VULNERABLE CODE #3
    INTEGER OVERFLOW
*/
void vulnerable_function_three(int x, int y)
{
    int total = x + y;
    //Secuirty Flaw (This Function might cause an overflow)
    printf("Total: ",total);
    //Solution:
    //Ensure the value is within the range before printf
    if(x > INT_MAX - y)
    {
        printf("Integer Overflow Detected! And Prevented!");
    }
    else
    {
        printf("Total",total);
    }
}
/*********************
    C FUNCTIONS
**********************/
/*
    VULNERABLE CODE #4 - 1
    gets()
*/
void vulnerable_function_four_one()
{
    char buffer[10];
    //Security Flaw (Reads Input without checking buffer bounds and can cause Buffer Overflows)
    gets(buffer);
    //Solution: use fgets()
    //fgets() function has a built in check for the buffer bounds
    fgets(buffer, sizeof(buffer), stdin);
}

/*
    VULNERABLE CODE #4 - 2
    strcpy() & stpcpy()
*/
void vulnerable_function_four_two()
{
    char destination[10];
    char source[] = "This is a long string.";
    //Security flaw (copys over the buffer contents without checking buffer bounderies, this causes a buffer overflow to occur)
    strcpy(destination,source);
    //Solution: use strncpy()
    //strncpy() checks for the bounderies of the destination buffer ensuring no buffer overflow occurs
    strncpy(destination,source,sizeof(destination) - 1);
}

/*
    VULNERABLE CODE #4 - 3
    strcat()
*/
void vulnerable_function_four_three()
{
    char buffer[10] = "Hello";
    //Security flaw (This function will concatenate the two strings without checking the size of the buffer capacity which will cause a buffer overflow)
    strcat(buffer," World!");
    //Solution: use strncat()
    //strncat() takes in the available space in the buffer as one of its arguments which ensures that the buffer overflow does not occur
    strncat(buffer," World!", sizeof(buffer) - strlen(buffer) - 1);
}

/*
    VULNERABLE CODE #4 - 4
    strcmp()
*/
void vulnerable_function_four_four()
{
    char entered_password[10] = "Dennis";
    char actual_password[10] = "notDennis";
    //Security Flaw (This Function is not unsafe by nature like the previous functions but careless use can lead to logic errors,
    //especially bad since this function is used for authentication)
    if(strcmp(entered_password,actual_password) == 0)
    {
        printf("Access Granted!\n Logging in User.... \n Logged In");
    }
    //Solution: Use of constant-time comparison function for security-sensitive data to mitigate timing attacks.
}

/*
    VULNERABLE CODE #4 - 5
    printf() * sprintf()
*/
void vulnerable_function_four_five()
{
    char buffer[10];
    //Security Flaw (Back to a less complicated function, this function does not check buffer size causing buffer overflow instances to occur)
    sprintf(buffer, "%s" , "This is a ver loong string!");
    //Solution: use snprintf()
    snprintf(buffer, sizeof(buffer), "%s", "this is a very long string!");
}


int main()
{
    /*VULNERABLE CODE #1 CODE*/
    vulnerable_function_one();

    /*VULNERABLE CODE #2 CODE*/
    vulnerable_function_two("input","Hi");

    /*VULNERABLE CODE #3 CODE*/ 
    vulnerable_function_three(5012313213123123123,123812930798123123213123);

    /*VULNERABLE CODE #4.1 FUNCTION CODE*/
    vulnerable_function_four_one();

    /*VULNERABLE CODE #4.2 FUNCTION CODE*/
    vulnerable_function_four_two();

    /*VULNERABLE CODE #4.3 FUNCTION CODE*/
    vulnerable_function_four_three();

    /*VULNERABLE CODE #4.4 FUNCTION CODE*/
    vulnerable_function_four_four();

    /*VULNERABLE CODE #4.5 FUNCTION CODE*/
    vulnerable_function_four_five();


    return 0;
}

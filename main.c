#include<stdio.h>

int main()
{   int size;
    printf("enter size of the array :");
    scanf("%d",&size);
    int num[size];
    for (int i=0;i<size;i++)
    {printf("enter number %d :",i+1);
     scanf("%d",&num[i]);
    }
    int numFind;
    int found=0;
    int count=0;
    int number;
    printf("enter no to be found: ");
    scanf("%d",&numFind);

    for (int i=0;i<size;i++)
    {
        if (num[i]==numFind)
         {count++;
          found=1;
          number=i;
         }

    }
    if (found==0)
        printf(" number not found ");
    else
        printf("number found\n %d appears %d times ",num[number],count);
    return 0;
}

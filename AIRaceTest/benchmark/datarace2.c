#include<pthread.h>
#include<stdio.h>
//#include <klee/klee.h>
int k=0;
void thread1(void)
{
//klee_make_symbolic(&i, sizeof(i), "i");
int i;

for(i=0;i<10;i++)
{
k++;
printf("thread2 %d\n",k);
sleep(1);
printf("1\n");
}
}

/*void thread2(void)
{

int i=0;
//klee_make_symbolic(&i, sizeof(i), "i");
if (i)

printf("%d\n",i);
else
printf("%d\n",i+1);
}*/

int main(void)
{
pthread_t id1;
int i;
pthread_create(&id1,NULL,(void *) thread1,NULL);
//pthread_create(&id2,NULL,(void *) thread2,NULL);
pthread_join(id1,NULL);
for(i=0;i<10;i++)
{
k++;
printf("thread1 %d\n",k);
sleep(1);
printf("0\n");
}
//pthread_join(id1,NULL);
//pthread_join(id2,NULL);
return 0;
}

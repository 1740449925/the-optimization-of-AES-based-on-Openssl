#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "./aes_x86core.c"
#include "./lib.c"
#include "./lib2.c"
#include<string.h>
#include<sys/time.h>
    // 分配存储密钥和明文的数组
unsigned char userkey[AES_BLOCK_SIZE]__attribute__((aligned(16)))={'\x00','\x11','\x22','\x33','\x44','\x55','\x66','\x77','\x88','\x99','\xaa','\xbb','\xcc','\xdd','\xee','\xff'};
unsigned char date[AES_BLOCK_SIZE]__attribute__((aligned(16)));
unsigned char encrypt[AES_BLOCK_SIZE]__attribute__((aligned(16)));
unsigned char plain[AES_BLOCK_SIZE]__attribute__((aligned(16)));
unsigned char date1[AES_BLOCK_SIZE];
unsigned char encrypt1[AES_BLOCK_SIZE];
unsigned char plain1[AES_BLOCK_SIZE];
AES_KEY en_key,de_key;

void RAND_bytes(unsigned char *a,int n){
     for (int i = 0; i < n; i++) {
        a[i] = rand() % 256;  // 生成0-255之间的随机数作为每个字节
    }
}
void test_ssl1(){
    memset((void*)encrypt1, 0, AES_BLOCK_SIZE);
    memset((void*)plain1, 0, AES_BLOCK_SIZE);
    AES_encrypt(date1, encrypt1, &en_key);    
    AES_decrypt(encrypt1, plain1, &de_key);    
}
void test_self2(){
    memset((void*)encrypt, 0, AES_BLOCK_SIZE);
    memset((void*)plain, 0, AES_BLOCK_SIZE);
    AES_encrypt_self2(date, encrypt, &en_key);      
    AES_decrypt_self2(encrypt, plain, &de_key);    
}
int main(){
   AES_set_encrypt_key_self2(userkey, AES_BLOCK_SIZE*8, &en_key);
   AES_set_decrypt_key_self2(userkey, AES_BLOCK_SIZE*8, &de_key);
   struct timeval start,self_end,self2_end,ssl_end;
    int i,j,k= 0;
    int cnt=1024*1024;
    gettimeofday(&start,NULL);
    for(;i<cnt;++i)
    {
    RAND_bytes(date, AES_BLOCK_SIZE);
        test_self2();
    }
   gettimeofday(&self_end,NULL);
   long timeuse_self=1000000*(self_end.tv_sec-start.tv_sec)+self_end.tv_usec-start.tv_usec;
    printf("%d次AES优化加解密过程CPU 占用的总时间：%lf s\n",cnt,(double)timeuse_self/1000000);
    for(;j<cnt;++j)
    {
    RAND_bytes(date, AES_BLOCK_SIZE);
    test_ssl1();
    }
   gettimeofday(&ssl_end,NULL);
   long timeuse_ssl=1000000*(ssl_end.tv_sec-self_end.tv_sec)+ssl_end.tv_usec-self_end.tv_usec;
    printf("%d次openssl_AES加解密过程CPU 占用的总时间：%lf s\n",cnt,(double)timeuse_ssl/1000000);
    return 0;

}
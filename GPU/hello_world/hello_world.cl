__kernel void hello()
{
 printf("Hello, I am core %u!\n", get_global_id(0));
};

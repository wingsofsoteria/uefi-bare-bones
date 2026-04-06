void __attribute__((section("kernel_init"))) (*const _kernel_init_start_)() = 0;

extern void* _kernel_init_end_;
void kernel_init_code()
{
  void (*const* fn)() = &_kernel_init_start_ + sizeof(char);
  while (fn != &_kernel_init_end_)
  {
    (*fn)();
    fn += (sizeof(char));
  }
}

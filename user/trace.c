#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include "kernel/param.h"
int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG];

  // Kiểm tra số lượng tham số
  // Cú pháp: trace [mask] [command] [args...]
  // Ví dụ: trace 32 grep hello README
  // argc phải ít nhất là 3
  if(argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')){
    fprintf(2, "Usage: %s mask command\n", argv[0]);
    exit(1);
  }

  // 1. Gọi system call trace() để thiết lập mask vào trong Kernel
  // atoi(argv[1]) chuyển chuỗi "32" thành số nguyên 32
  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }
  
  // 2. Chuẩn bị tham số cho lệnh tiếp theo (ví dụ: grep hello README)
  for(i = 2; i < argc && i < MAXARG; i++){
    nargv[i-2] = argv[i];
  }
  nargv[i-2] = 0; // Kết thúc mảng bằng null

  // 3. Chạy lệnh đó bằng exec
  // Khi exec chạy, nó sẽ thay thế chương trình "trace" hiện tại bằng chương trình mới (ví dụ "grep")
  // NHƯNG, vì trace_mask nằm trong struct proc của kernel, nó vẫn được giữ nguyên.
  exec(nargv[0], nargv);
  
  // Nếu exec chạy thành công, dòng này sẽ không bao giờ tới.
  // Nếu tới đây nghĩa là lỗi.
  exit(0);
}
# Lab7_g6

> 來源:review/predict/Lab7_predict.md

## 第 6 題（可能性：中低）驗證觀念：kernel persistence 與 MAP_PRIVATE vs MAP_SHARED

### 題目
1. `writer.c`：建立 shared memory，寫入 `"original data"` 後直接結束（**不 unlink**）。用 `ls /dev/shm` 證明 process 結束後物件仍存在。
2. `reader.c`：先用 `MAP_PRIVATE`（PROT_READ|PROT_WRITE）映射並修改內容，再用 `MAP_SHARED` 映射同一個物件，印出內容，證明 MAP_PRIVATE 的修改**不會**影響到 shared memory；接著用 MAP_SHARED 修改。再執行一次 reader 證明 MAP_SHARED 的修改有留下來。
3. `cleaner.c`：印出最終內容並 `shm_unlink`，之後 `ls /dev/shm` 看不到物件。

輸出範例：
```
$ ./writer
writer: wrote "original data" and exit (no shm_unlink)
$ ls /dev/shm
persistShm_lab7p
$ ./reader
[PRIVATE] before: original data
[PRIVATE] after : modified by MAP_PRIVATE
[SHARED ] content: original data
[SHARED ] after : modified by MAP_SHARED
$ ./reader
[PRIVATE] before: modified by MAP_SHARED
[PRIVATE] after : modified by MAP_PRIVATE
[SHARED ] content: modified by MAP_SHARED
[SHARED ] after : modified by MAP_SHARED
$ ./cleaner
cleaner: final content = modified by MAP_SHARED
cleaner: shm_unlink done
$ ls /dev/shm
```

### 變形自哪裡 / 考什麼
- 7-2 p.7：objects 在 `/dev/shm`；**kernel persistence**——沒有 process 開著也會存在，直到關機或 unlink。
- 7-2 p.18：`MAP_PRIVATE`「Modifications to mapped data are private」、`MAP_SHARED`「visible to other processes」——**簡報用紅框圈起來**。
- 符合共通邏輯 2「Exercise 常用來驗證一個觀念」。

### 為什麼可能性中低
是純觀念驗證，程式量小；比較可能以「口頭問答 / 修改一個參數觀察結果」形式出現，而不是完整 50 分題。但紅框內容值得準備。

### 參考答案
`writer.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, SMSIZE) == -1) { perror("ftruncate"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    strcpy(ptr, "original data");
    printf("writer: wrote \"%s\" and exit (no shm_unlink)\n", ptr);

    munmap(ptr, SMSIZE);
    close(fd);
    return 0;      // process 結束了，但 shared memory 物件還在（kernel persistence）
}
```
`reader.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    /* (1) MAP_PRIVATE：改了只影響自己（copy-on-write），不會寫回 shared memory */
    char *priv = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (priv == MAP_FAILED) { perror("mmap private"); return 1; }
    printf("[PRIVATE] before: %s\n", priv);
    strcpy(priv, "modified by MAP_PRIVATE");
    printf("[PRIVATE] after : %s\n", priv);

    /* (2) MAP_SHARED：看到的是真正 shared memory 的內容 */
    char *shared = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) { perror("mmap shared"); return 1; }
    printf("[SHARED ] content: %s\n", shared);

    strcpy(shared, "modified by MAP_SHARED");
    printf("[SHARED ] after : %s\n", shared);

    munmap(priv, SMSIZE);
    munmap(shared, SMSIZE);
    close(fd);
    return 0;
}
```
`cleaner.c`
```c
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }
    printf("cleaner: final content = %s\n", ptr);
    munmap(ptr, SMSIZE);
    close(fd);
    if (shm_unlink(NAME) == -1) { perror("shm_unlink"); return 1; }
    printf("cleaner: shm_unlink done\n");
    return 0;
}
```

### ✅ 已驗證
```
writer: wrote "original data" and exit (no shm_unlink)
$ ls /dev/shm
persistShm_lab7p
--- reader terminal ---
[PRIVATE] before: original data
[PRIVATE] after : modified by MAP_PRIVATE
[SHARED ] content: original data              ← MAP_PRIVATE 的修改沒有寫回
[SHARED ] after : modified by MAP_SHARED
--- reader again ---
[PRIVATE] before: modified by MAP_SHARED      ← MAP_SHARED 的修改有留下
...
--- cleaner ---
cleaner: final content = modified by MAP_SHARED
cleaner: shm_unlink done
$ ls /dev/shm                                  ← 已消失
$ ./reader
shm_open: No such file or directory
```

### 容易踩的坑
- `MAP_PRIVATE` + `PROT_WRITE` 可以搭配 `O_RDONLY` 的 fd（copy-on-write）；但 `MAP_SHARED` + `PROT_WRITE` 一定要 `O_RDWR` 開，否則 `mmap` 回 `Permission denied`（兩者皆已實測）。
- 用 `PROT_READ` 映射卻去寫 → **Segmentation fault**。
- 同理，message queue 也有 kernel persistence：第 4 題中 sender 結束後 `/dev/mqueue/prioQueue_lab7p` 仍在、`QSIZE:60`。

# practice 題目索引

## 看到題目怎麼找資料夾

**步驟:① 用關鍵字認出 Lab → ② 用特徵挑最像的題目 → ③ 打開資料夾,從那份程式改,不要從零寫。**

### ① 判斷是哪個 Lab

| 題目出現這些字 | Lab |
|---|---|
| shell、`.sh`、bash、echo、read、`$1` | **Lab 1** |
| system call、syscall、`SYSCALL_DEFINE`、462、dmesg、重編 kernel | **Lab 2** |
| 只能用 `<unistd.h>`、open/read/write/close、fd、權限、strace | **Lab 3** |
| library、`.a`、`.so`、static、dynamic、shared、`ar`、`ldd` | **Lab 4** |
| kernel module、`.ko`、insmod、rmmod、module_param、EXPORT_SYMBOL | **Lab 5** |
| process、fork、wait、PID/PPID、process tree、exec | **Lab 6** |
| message queue、mq_、shared memory、shm_、mmap、sender/receiver、producer/consumer | **Lab 7** |

### ② 挑最像的題目

| Lab | 題目特徵 → 資料夾 |
|---|---|
| 1 | 猜數字 → `ex`、`g1`;範圍/次數用參數給 → `g2`;選單、計算機、case → `g3`;兩支 script 用 pipe 串接 → `g4`;命令列數字算總和/最大值 → `g5`;帳號密碼、字串比較 → `g6` |
| 2 | 一個參數印 n 次 → `ex2`、`g1`;兩參數相加回傳 → `g2`;kernel 計算後回傳(累加) → `g3`;問要改哪些檔案、指令順序 → `g4`;新增第二個 syscall → `g5`;參數不合法回傳錯誤 → `g6` |
| 3 | 只用 unistd.h 印數字/字串 → `ex`、`g1`;從鍵盤讀數字(fd 0) → `g2`;複製檔案(system call) → `g3`;複製檔案(fopen/fread) → `g6`;建檔指定權限 → `g4`;write vs fwrite 速度、strace → `g5` |
| 4 | sort + static/dynamic 比大小 → `ex`、`g3`;降冪或 N 個數 → `g1`;多個 .c 包一個 library(max/min/avg) → `g2`;更新 library 比較 static/dynamic → `g4`;四則運算 → `g5`;字串反轉/轉大寫 → `g6` |
| 5 | 生日轉時間戳、活了幾年 → `ex`;倒數、距離某天多久 → `g1`;參數有時分秒 → `g2`;export **函式**、兩日期相差幾天 → `g3`;足歲、下次生日 → `g4`;星期幾 → `g5`;單一 module、第幾天、閏年 → `g6` |
| 6 | global/local + fork → `ex1`(parent 也改值 → `g2`);process tree → `ex2`(換一棵樹 → `g1`,一條直線 → `g5`);exec、execlp、執行 ls → `g3`;迴圈產生多個 child → `g4`;fork 幾次有幾個 process → `g6` |
| 7 | message queue 單向傳字串 → `ex1`;雙向、計算機 → `g1`;priority → `g4`;shared memory 一則訊息 → `ex2`;多筆訊息 → `g2`;傳數字並計算 → `g3`;兩者一起用 → `g5`;MAP_PRIVATE/SHARED、資料會不會保留 → `g6` |

資料夾全名是 `Lab{編號}_{上表代號}`,例如 Lab 6 的 `g3` 就是 `practice/Lab6_g3/`。

### ③ 打開資料夾後

1. 先看 `README.md` 的「題目」,和考卷對照,找出不同的地方。
2. 複製程式到自己的資料夾,只改不同的部分。
3. 考前看過 README 最後的「容易踩的坑」。

**都不太像?** 先看 `LabN_ex`(原題)。所有猜題都是從原題變形來的,骨架一定用得上。


## 總表

- `LabN_ex*`:課堂 Exercise 原題(簡報最後的練習)
- `LabN_gK`:上機考猜題(依可能性排序,g1 最可能)
- 每個資料夾裡的 `README.md` 有完整題目、參考答案說明與容易踩的坑;離線速查請開 `題目索引.pdf` 或 `題目索引.html`。

## Lab 1:Shell Script

| 資料夾 | 題目 | 可能性 | 檔案 |
|---|---|---|---|
| `Lab1_ex` | 猜數字遊戲 Number Guessing Game | 原題 | `ex.sh` |
| `Lab1_g1` | 【原題重現】Number Guessing Game（1~10，範圍逐步縮小） | 高 | `NGG.sh` |
| `Lab1_g2` | 【加料版猜數字】範圍與次數改用參數給，並統計猜了幾次 | 高 | `guess.sh` |
| `Lab1_g3` | 【case + function + expr】選單式計算機 | 中高 | `calc.sh` |
| `Lab1_g4` | 【Pipe】兩支 script 串接：產生分數 \| 判定等第並統計 | 中 | `gen.sh`, `grade.sh` |
| `Lab1_g5` | 【命令列參數 + expr】三個數字：加總、最大值、階乘 | 中 | `nums.sh` |
| `Lab1_g6` | 【字串比較】帳號密碼檢查（最多 3 次） | 中低 | `login.sh` |

## Lab 2:Adding a New System Call

| 資料夾 | 題目 | 可能性 | 檔案 |
|---|---|---|---|
| `Lab2_ex1` | Exercise-1(60 分):完成範例,新增 hello syscall | 原題 | `hello.c`, `test.c` |
| `Lab2_ex2` | Exercise-2(40 分):SYSCALL_DEFINE1 印 n 次 hello | 原題 | `hello.c`, `test.c` |
| `Lab2_g1` | Exercise 2 原題:傳入 n,印 n 次 hello | 高 | `hello.c`, `test.c` |
| `Lab2_g2` | SYSCALL_DEFINE2:兩個參數相加,並回傳結果 | 高 | `hello.c`, `test.c` |
| `Lab2_g3` | 用回傳值把計算結果帶回 user space | 中~高 | `hello.c`, `test.c` |
| `Lab2_g4` | 流程/填空/口試題:新增 syscall 要改哪些檔案 | 中 |  |
| `Lab2_g5` | 新增第二個 syscall(編號 463):兩數相加 | 中低 | `hello.c`, `test.c` |
| `Lab2_g6` | 參數檢查:不合法時回傳錯誤 | 低 | `hello.c`, `test.c` |

## Lab 3:System Call vs APIs

| 資料夾 | 題目 | 可能性 | 檔案 |
|---|---|---|---|
| `Lab3_ex` | 只用 <unistd.h> 印出 integer | 原題 | `ex`, `ex.c` |
| `Lab3_g1` | 只用 <unistd.h> 印出字串與整數陣列（含負數） | 高 | `q1.c` |
| `Lab3_g2` | 只用 <unistd.h> 從 stdin（fd 0）讀入兩個整數，輸出和 | 高 | `q2.c` |
| `Lab3_g3` | 只用 system call 複製檔案（自製 cp） | 中高 | `q3.c` |
| `Lab3_g4` | 用 open() 指定權限建立檔案，把整數寫進檔案 | 中 | `q4.c` |
| `Lab3_g5` | write() vs fwrite() 一次寫 1 byte，比較時間並用 strace 觀察 | 中 | `q5_fwrite.c`, `q5_write.c` |
| `Lab3_g6` | 用 C library（fopen/fread/fwrite/fclose）改寫檔案複製 | 低 | `q6.c` |

## Lab 4:Library

| 資料夾 | 題目 | 可能性 | 檔案 |
|---|---|---|---|
| `Lab4_ex` | Exercise 1 static library + Exercise 2 shared library(sort) | 原題 | `ex1`, `ex1.c`, `ex_dy`, `exlib.a`, `exlib.h`, `exlib.so`, `sort.c`, `sort.o` |
| `Lab4_g1` | 降冪排序 library（10 個不重複亂數） | 高 | `exercise.c`, `exlib.h`, `sort.c` |
| `Lab4_g2` | 統計函式庫 max / min / average（多個 .o 包成一個 library） | 高 | `average.c`, `main.c`, `max.c`, `min.c`, `mylib.h` |
| `Lab4_g3` | 原題重現 + ldd 驗證 + 刪除 library 實驗 | 中高 | `exercise.c`, `exlib.h`, `sort.c` |
| `Lab4_g4` | 更新 library 後觀察 static / dynamic 行為差異 | 中 | `exercise.c`, `exlib.h`, `sort_v1.c`, `sort_v2.c` |
| `Lab4_g5` | 擴充投影片範例 — 四則運算 library | 中 | `add.c`, `div.c`, `lib.h`, `main.c`, `mul.c`, `sub.c` |
| `Lab4_g6` | 字串函式庫 reverse + to_upper | 低 | `main.c`, `reverse.c`, `strlib.h`, `upper.c` |

## Lab 5:Kernel Module

| 資料夾 | 題目 | 可能性 | 檔案 |
|---|---|---|---|
| `Lab5_ex` | date2timestamp + myage | 原題 | `Makefile`, `Module.symvers`, `d2ts.c`, `d2ts.ko`, `d2ts.mod`, `d2ts.mod.c`, `d2ts.mod.o`, `d2ts.o`, `modules.order`, `myage.c`, `myage.ko`, `myage.mod`, `myage.mod.c`, `myage.mod.o`, `myage.o` |
| `Lab5_g1` | 倒數計時 | 高 | `Makefile`, `countdown.c`, `deadline.c` |
| `Lab5_g2` | 加上時分秒：活了幾年幾天幾時幾分幾秒 | 高 | `Makefile`, `date2timestamp.c`, `myage.c` |
| `Lab5_g3` | Export 函式：兩個日期相差幾天 | 中高 | `Makefile`, `datelib.c`, `daydiff.c` |
| `Lab5_g4` | 精確足歲＋下次生日倒數 | 中 | `Makefile`, `birthday.c`, `date2timestamp.c` |
| `Lab5_g5` | 出生那天是星期幾 | 中低 | `Makefile`, `date2timestamp.c`, `weekday.c` |
| `Lab5_g6` | 單一 module：該年第幾天、是否閏年 | 低 | `Makefile`, `dayofyear.c` |

## Lab 6:Process Control

| 資料夾 | 題目 | 可能性 | 檔案 |
|---|---|---|---|
| `Lab6_ex1` | Exercise 1(40 分):fork 後 global / local 變數 | 原題 | `ex1`, `ex1.c` |
| `Lab6_ex2` | Exercise 2(50 分):5 個 process 的樹 | 原題 | `ex2`, `ex2.c` |
| `Lab6_g1` | 換一棵 process tree（6 個 process） | 高 | `q1_tree.c` |
| `Lab6_g2` | Ex1 改成 parent 和 child 都遞增，加上 heap 變數 | 高 | `q2_vars.c` |
| `Lab6_g3` | fork–exec，child 用 execlp 執行 ls -l，parent 等待後印訊息 | 中高 | `q3_exec.c` |
| `Lab6_g4` | 用迴圈 fork N 個 child（扇形樹），parent wait N 次並印出回收的 child PID | 中 | `q4_fan.c` |
| `Lab6_g5` | 鏈狀 process tree A → B → C → D | 中 | `q5_chain.c` |
| `Lab6_g6` | fork() 連續呼叫 n 次，總共有幾個 process？ | 低～中 | `q6_count.c` |

## Lab 7:IPC(Message Queue / Shared Memory)

| 資料夾 | 題目 | 可能性 | 檔案 |
|---|---|---|---|
| `Lab7_ex1` | Exercise-Lab1(50 分):Message Queue sender / receiver | 原題 | `receiver`, `receiver.c`, `sender`, `sender.c` |
| `Lab7_ex2` | Exercise 2(50 分):Shared Memory producer / consumer | 原題 | `consumer`, `consumer.c`, `producer`, `producer.c` |
| `Lab7_g1` | 雙向 Message Queue：計算機 client / server | 高 | `client.c`, `server.c` |
| `Lab7_g2` | Shared Memory 寫入多筆訊息直到 exit | 高 | `consumer.c`, `producer.c` |
| `Lab7_g3` | Shared Memory 傳整數陣列，consumer 計算 | 中高 | `consumer.c`, `producer.c` |
| `Lab7_g4` | 依 Priority 接收 + mq_getattr + O_NONBLOCK | 中 | `receiver.c`, `sender.c` |
| `Lab7_g5` | 結合兩者：shm 放資料、message queue 當通知 | 中 | `consumer.c`, `producer.c` |
| `Lab7_g6` | 驗證觀念：kernel persistence 與 MAP_PRIVATE vs MAP_SHARED | 中低 | `cleaner.c`, `reader.c`, `writer.c` |

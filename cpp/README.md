# 有限区间验证程序

本目录提供实际可编译的 C++17 程序。它对给定范围内的**每个奇数** `n ∈ [7, limit]` 搜索
`n = 3 + p + q`，其中 `p`、`q` 是素数。每次成功都给出精确整数见证。

这只是有限区间实验，**不是弱哥德巴赫猜想的无限范围证明，也不是圆法主弧／次弧估计**。
固定第一个素数为 3 是一个更强的有限搜索条件；如果搜索失败，只说明没有找到这种形式的见证，
不能称为弱哥德巴赫反例。程序以退出码 2 和 `RESTRICTED_SEARCH_FAILURE` 区分这种情况。

## 算法与 SIMD 的实际范围

1. 使用普通、标量 Eratosthenes 筛生成 `0..limit` 的素数表。
2. 对每个奇数令 `E = n - 3`，顺序枚举素数 `p ≤ E/2`，检查 `E-p` 是否为素数。
3. AVX2 实现一次读取 8 个 `p`，通过 8 路整数 gather 读取素数标志，返回最早成功的候选；剩余不足 8 个使用标量循环。
4. 启动时检测 CPU 的 AVX2 能力。`auto` 自动选择；`scalar` 强制标量；`simd` 在不支持时明确报错。

**筛本身没有 SIMD 加速**。AVX2 只用于候选查询，且不承诺更快；对容易找到见证的小范围，标量实现可能更快。
AVX2 使用函数级 `target("avx2")`，构建时不要加全局 `-mavx2` 或 `-march=native`，避免使标量回退失效。
支持 GNU/Clang x86 的 AVX2 分派；其他平台编译为标量实现。

`limit` 为包含端点的上界，允许 `7..10000000`。主要内存为 32 位素数标记表、素数数组与每个奇数的一个见证。
所有 gather 索引非负且在表内。只用整数判断素性和求和，时间统计使用浮点数但不参与正确性判断。

## 构建与复核

在本目录执行（Windows PowerShell 可使用 `./verify.exe`）：

```text
g++ -O3 -std=c++17 -Wall -Wextra -Wpedantic verify.cpp -o verify.exe
./verify.exe --self-test
python check.py ./verify.exe
./verify.exe --limit 1000000 --mode simd --cross-check
./verify.exe --limit 10001 --mode scalar --cross-check --csv witnesses_10001.csv
```

`check.py` 使用独立的 Python 试除法核验导出的全部 4,998 条见证，测试错误参数和区间端点。
内置自检也将 `0..10001` 的全部 10,002 个素性标志与试除法比较，并在可用时逐一比较 SIMD 与标量结果。
`--cross-check` 对指定全区间比较两条实现的每一条见证，而非只比较计数。

CSV 格式为 `n,a,b,c`，每行对应 `n=a+b+c`。输出仅包含成功见证；必须同时检查程序退出码、
`checked_odds`、`found_witnesses` 和 `result`，不能只看 CSV 文件存在。

输出中的 `search_ms` 不包含建筛、CSV 写入或另一实现的交叉检查。
它包含候选搜索、结果保存与校验和计算；`other_search_ms` 是后运行实现的同口径时间。
缓存与运行顺序会影响这些数值，单次运行不能用作可靠的加速比结论。

## 可信边界

这些是普通机器程序运行结果，依赖编译器、运行时和硬件正确性。
此 C++ 程序尚未在 Lean 中形式化验证，CSV 也尚未导入 Lean 内核逐条认证。
独立试除检查只覆盖明示的小范围；百万上界的完整验证由筛表和两种查询实现完成。
发布任何结果时应保留源码、构建命令、编译器版本、输入上界、退出码和实际运行日志。

## 随附实测证据（2026-11-19）

- `evidence/environment.txt`：构建命令、编译器版本和 CPU 型号。
- `evidence/build.log`：警告开启的优化构建退出码 0。
- `evidence/checks.log`：全部自检、Python 独立试除、错误参数与端点检查通过。
- `evidence/million.log`：上界 1,000,000；499,997 个奇数全部找到见证；每条 SIMD 与标量见证一致。
- `witnesses_10001.csv`：上界 10,001 的全部 4,998 条具体见证；对应运行日志为 `evidence/witnesses_10001.log`。
- `evidence/SHA256SUMS.txt`：源码、检查脚本、二进制与 CSV 的 SHA-256 摘要。

百万上界的记录中，AVX2 搜索耗时 3.401 ms、后运行的标量查询耗时 3.065 ms。
这是本机的一次实测：**未观察到 SIMD 加速**，且交叉检查的运行顺序可能影响缓存。
程序及数据的价值在于复现与核验，不在于把这次结果宣传为性能突破。

# Weak Goldbach: Lean 基础证明与有限区间验证

> 当前验证状态（2026-11-19）：C++ 构建、百万区间检查与独立试除均已通过。Lean 源码已完成，编译验收正在进行；在提交实际 Lean 构建日志前，不应将源码视为已经通过内核检查的交付结果。

这是从一个未完成的圆法草稿整理出的**可复核阶段性项目**：包含 Lean 4 基础引理、一个小范围内核证明，以及 C++/AVX2 有限区间实验。它没有完成弱哥德巴赫定理的圆法形式化，也没有把主弧、次弧估计包装成已证明结果。

本项目针对弱哥德巴赫问题。PFR（Polynomial Freiman–Ruzsa）属于另一项数学问题，不是本项目的别名。

## 可以直接审查的成果

| 内容 | 源码 | 具体含义 |
| --- | --- | --- |
| 指数和基础界 | [Foundations.lean](WeakGoldbach/Foundations.lean) | `norm_expSum_le_card`：素数指数和的范数不超过求和项数；进一步不超过 `N`。这是三角不等式界，没有抵消估计。 |
| 主弧／次弧划分 | [Foundations.lean](WeakGoldbach/Foundations.lean) | 显式定义有理数邻域，证明覆盖整个单位区间且交集为空。 |
| 小范围形式化证明 | [FiniteCertificate.lean](WeakGoldbach/FiniteCertificate.lean) | 对每个满足 `7 ≤ n ≤ 101` 的奇数给出素数三元组，使用普通 `decide` 并由 Lean 内核检查。 |
| 有限与无限尾部拼接 | [Foundations.lean](WeakGoldbach/Foundations.lean) | `finite_tail_splice` 证明两段假设如何合并；无限尾部仍是明确假设。 |
| 分析估计的规格接口 | [Targets.lean](WeakGoldbach/Targets.lean) | 明示主项、误差、统一常数和阈值的 `Prop` 定义；**没有提供这些命题的证明**。 |
| 实际 SIMD 代码 | [verify.cpp](cpp/verify.cpp) | 标量素数筛 + AVX2 八路候选查询，带 CPU 检测、标量回退和逐条交叉检查。 |

百万范围的实际运行结果：对 `[7, 1,000,000]` 内全部 **499,997 个奇数**，找到 `n = 3 + p + q` 的素数见证；SIMD 与标量返回的每一条见证相同。另有至 `10,001` 的 **4,998 条 CSV 见证**经过独立 Python 试除核验。详见 [运行日志](cpp/evidence/million.log)、[具体见证](cpp/witnesses_10001.csv) 和 [检查日志](cpp/evidence/checks.log)。

这些 C++ 结果是普通机器计算，尚未通过 Lean 内核认证。Lean 中的 `7..101` 结论使用独立的有限证书证明。两条证据链的可信边界不同。

## 三个可核验的 Lean 声明

以下声明的实现位于上述 `.lean` 文件。

```lean
theorem norm_expSum_le_card (N : ℕ) (α : ℝ) :
    ‖expSum N α‖ ≤ ((primesBelow N).card : ℝ)

theorem majorArc_union_minorArc (Q : ℕ) (δ : ℝ) :
    majorArc Q δ ∪ minorArc Q δ = unitInterval

theorem weakGoldbach_upTo101 (n : ℕ)
    (hmin : 7 ≤ n) (hmax : n ≤ 101) (hodd : n % 2 = 1) :
    HasPrimeTriple n
```

这些是本项目新增的基础成果，不声称它们就是原草稿缺失的深层圆法引理。

## 复现

Lean 固定为 **4.5.0**，Mathlib 固定为提交
`feec58a7ee9185f92abddcf7631643b53181a7d3`（v4.5.0）。传递依赖由 `lake-manifest.json` 锁定。

已安装 [elan](https://github.com/leanprover/elan) 后，在项目根目录运行：

```text
lake exe cache get Mathlib/Data/Nat/Prime.lean Mathlib/Analysis/SpecialFunctions/Trigonometric/Basic.lean Mathlib/Analysis/SpecialFunctions/Log/Basic.lean
lake build
lake env lean scripts/Audit.lean
python scripts/check_proofs.py
```

C++ 需要支持 C++17 的 GNU/Clang 编译器和 Python 3：

```text
g++ -O3 -std=c++17 -Wall -Wextra -Wpedantic cpp/verify.cpp -o cpp/verify.exe
python cpp/check.py cpp/verify.exe
cpp/verify.exe --limit 1000000 --mode auto --cross-check
```

Linux/macOS 可将输出名改为 `cpp/verify`；没有 AVX2 的机器使用标量路径，并明确输出交叉检查跳过原因。完整说明见 [cpp/README.md](cpp/README.md)。

## 已知范围和后续工作

- 尚未给出独立、精确的奇异级数／积分主项，尚未证明主弧误差或次弧抵消界。
- 尚未形式化积分恒等式、表示数的正性、明确阈值及其与有限验证区间的衔接。
- `n = 3 + p + q` 是有限验证使用的充分条件；搜索失败不能称为弱哥德巴赫反例。
- 该实现的素数筛本身是标量；SIMD 仅用于候选查询。一次百万范围实测为 AVX2 3.401 ms、标量对照 3.065 ms，**未观察到加速**，不据此宣传性能优势。
- 本项目不声称有未展示的私有仓库、NDA 成果、Devin 检索日志或已完成的完整数学证明。

原始草稿中未定义的弧、空泛的存在量词、无效的 Big-O 写法及无关 PR 引用，已在 [审查说明](docs/audit.md) 中记录。

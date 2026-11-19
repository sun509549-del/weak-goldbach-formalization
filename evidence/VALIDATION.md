# 验证记录

记录日期：2026-11-19（Asia/Taipei）。这是实际执行记录，不是预计结果。

## Lean

- 编译器：`Lean (version 4.5.0, commit 1a3021f98e55, Release)`，官方 Windows 发行版。
- Mathlib：`feec58a7ee9185f92abddcf7631643b53181a7d3`；其余依赖固定在根目录 `lake-manifest.json`。
- `lake build`：退出码 **0**，四个模块成功编译，见 [lean-build.log](lean-build.log)。
- `lake env lean scripts/Audit.lean`：退出码 **0**，见 [lean-axioms.log](lean-axioms.log)。
- `python scripts/check_proofs.py`：通过，四个项目 Lean 源文件无证明占位符、自定义公理或原生计算证明捷径。

`weakGoldbach_upTo101` 证明包含端点的 `[7,101]` 内全部 48 个奇数都存在素数三元组。证书通过普通 `decide` 生成并接受 Lean 内核检查；该结论不依赖 C++ 的程序输出。

被审计声明只依赖所列的 Lean/Mathlib 标准逻辑公理（`propext`、`Classical.choice`、`Quot.sound` 的子集），没有 `sorryAx`。这不表示“零公理”，也不表示已证明无限范围的弱哥德巴赫定理。

本地构建日志中的 ProofWidgets 预构建标签警告来自浅克隆依赖；它未导致构建失败，也不影响这些声明的类型检查。

## 有限计算

- C++17 优化构建通过，警告开启且无编译警告。
- 内置素数筛与独立试除比较：10,002 个整数全部一致。
- Python 独立检查：4,998 条 CSV 素数三元组、10 个错误输入和 5 个区间端点测试全部通过。
- 百万范围：499,997 个奇数全部找到 `n=3+p+q` 见证，SIMD/标量逐条一致。
- 数值校验和：`6535821832762130111`。
- 具体命令、CPU 和编译器见 [environment.txt](../cpp/evidence/environment.txt)；原始日志见 [checks.log](../cpp/evidence/checks.log) 和 [million.log](../cpp/evidence/million.log)。

有限计算依赖普通编译器与硬件执行；百万区间结果尚未转为 Lean 内核证明。AVX2 只用于候选查询；本次单次运行没有观察到性能提升。

## 仍未完成

`MajorArcApproximation` 与 `UniformMinorArcBound` 是待研究的命题接口，没有证明。奇异级数、积分恒等式、正性、无限尾部与有效阈值仍需数学工作；`finite_tail_splice` 只证明明确假设之间的逻辑衔接。

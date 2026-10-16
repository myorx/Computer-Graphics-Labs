# Lab 4：3D Gaussian Splatting 场景剪枝

本实验比较不透明度阈值和尺度阈值对高斯点数量、模型体积、渲染速度及图像质量的影响。实验方法、结果和图表见 [实验报告](report.md)；[PDF 版本](report.pdf)供离线阅读。

## 与官方项目的关系

`train.py`、`render.py`、`full_eval.py` 基于 [3D Gaussian Splatting 官方实现](https://github.com/graphdeco-inria/gaussian-splatting)。它们导入该项目的 `scene`、`gaussian_renderer`、`arguments` 和 `utils` 等模块，本目录并未包含这些模块或 CUDA 扩展。复现实验须先按官方说明建立完整环境，并遵守 [原项目许可](LICENSE-3DGS.md)；这些文件不可作为独立程序直接运行。

## 复现概览

1. 准备官方 3DGS 工程、兼容的 CUDA/PyTorch 环境，以及 Tanks & Temples 的 `truck` 数据。依赖版本和数据准备以官方项目说明为准。
2. 在官方工程中训练基准模型，使 `output/truck_base/point_cloud/iteration_30000/point_cloud.ply` 和 `output/truck_base/cfg_args` 可用。
3. 将本目录中的 `my_prune.py`、`my_prune_scale.py` 放入官方工程根目录，从该目录运行。脚本会读取上述基准模型，并在 `output/` 下生成剪枝模型。
4. 使用官方渲染流程生成对比图后，再运行 `eval_psnr.py` 计算质量指标；`plot_results.py` 可根据脚本内的实验数据重绘统计图。

脚本中的 `output/truck_base`、渲染目录及图像文件名是本实验使用的约定；使用其他场景或迭代次数时需相应调整。仓库不包含训练数据、点云模型和完整的官方工程。

## 文件

| 文件 | 用途 |
| --- | --- |
| `my_prune.py` | 按不透明度阈值剪枝 |
| `my_prune_scale.py` | 按高斯元尺度剪枝 |
| `eval_psnr.py` | 计算 PSNR/SSIM 并绘制质量曲线 |
| `plot_results.py` | 绘制数量、速度和体积统计图 |
| `report.md` / `report.pdf` | 实验过程、数据及结论 |

本目录中带有 Inria/GRAPHDECO 版权声明的代码受 [原项目许可](LICENSE-3DGS.md) 约束，限非商业研究与评估用途。

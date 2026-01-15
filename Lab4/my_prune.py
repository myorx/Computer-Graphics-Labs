import torch
import os
import shutil
from scene.gaussian_model import GaussianModel

def prune_gaussians(base_model_path, threshold):
    # 1. 加载官方模型结构
    gaussians = GaussianModel(sh_degree=3)
    
    # 定位你刚刚训练出来的基准模型路径
    ply_path = os.path.join(base_model_path, "point_cloud", "iteration_30000", "point_cloud.ply")
    if not os.path.exists(ply_path):
        print(f"错误：找不到基准模型文件 {ply_path}")
        return

    # 加载点云数据到内存
    gaussians.load_ply(ply_path)
    
    # 记录原始数量
    orig_points = gaussians.get_xyz.shape[0]
    
    # 2. 【核心算法】：执行剪枝
    # 实验手册要求：剔除不透明度低于预设阈值 opacity_threshold 的元
    # gaussians.get_opacity 获取的是经过 sigmoid 激活后的 [0, 1] 之间的值
    opacities = gaussians.get_opacity
    keep_mask = (opacities > threshold).squeeze()
    
    # 根据掩码(Mask)剔除对应的张量索引
    # 必须同步更新所有属性，否则模型会损坏
    gaussians._xyz = gaussians._xyz[keep_mask]
    gaussians._features_dc = gaussians._features_dc[keep_mask]
    gaussians._features_rest = gaussians._features_rest[keep_mask]
    gaussians._opacity = gaussians._opacity[keep_mask]
    gaussians._scaling = gaussians._scaling[keep_mask]
    gaussians._rotation = gaussians._rotation[keep_mask]
    
    # 记录新数量
    new_points = gaussians.get_xyz.shape[0]
    
    # 3. 保存简化后的模型
    # 为了让 render.py 能够识别，我们需要模拟官方的目录结构
    out_folder = f"output/pruned_{threshold}"
    save_path = os.path.join(out_folder, "point_cloud", "iteration_30000")
    os.makedirs(save_path, exist_ok=True)
    
    gaussians.save_ply(os.path.join(save_path, "point_cloud.ply"))
    
    # 同时拷贝配置文件，这是 render.py 运行必须的
    cfg_src = os.path.join(base_model_path, "cfg_args")
    if os.path.exists(cfg_src):
        shutil.copy(cfg_src, out_folder)
    
    print("-" * 50)
    print(f"剪枝完成！阈值: {threshold}")
    print(f"原始高斯数量: {orig_points}")
    print(f"剩余高斯数量: {new_points}")
    print(f"点数缩减率: {(1 - new_points/orig_points)*100:.2f}%")
    print(f"新模型保存至: {out_folder}")

if __name__ == "__main__":
    # 按照实验手册要求的阈值进行循环处理
    thresholds = [0.01, 0.05, 0.1, 0.2]
    for t in thresholds:
        prune_gaussians("output/truck_base", t)

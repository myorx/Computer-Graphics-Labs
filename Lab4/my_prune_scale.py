import torch
import os
import shutil
from scene.gaussian_model import GaussianModel

def prune_by_scale(base_model_path, scale_threshold):
    # 1. 加载基准模型
    gaussians = GaussianModel(sh_degree=3)
    ply_path = os.path.join(base_model_path, "point_cloud", "iteration_30000", "point_cloud.ply")
    gaussians.load_ply(ply_path)
    
    orig_points = gaussians.get_xyz.shape[0]

    # 2. 【核心算法】：执行尺度剪枝
    # gaussians.get_scaling 内部执行了 torch.exp(self._scaling)
    # 结果是一个 [N, 3] 的张量，代表每个点的三个轴向长度
    scales = gaussians.get_scaling
    
    # 获取每个高斯元三个轴中最长的那一个轴
    # 如果最长的轴都小于阈值，说明这个点在空间中微小到可以忽略
    max_scales = torch.max(scales, dim=1).values
    
    # 生成掩码：保留最大边长大于阈值的元
    scale_mask = (max_scales > scale_threshold)
    
    # 执行物理剔除
    gaussians._xyz = gaussians._xyz[scale_mask]
    gaussians._features_dc = gaussians._features_dc[scale_mask]
    gaussians._features_rest = gaussians._features_rest[scale_mask]
    gaussians._opacity = gaussians._opacity[scale_mask]
    gaussians._scaling = gaussians._scaling[scale_mask]
    gaussians._rotation = gaussians._rotation[scale_mask]
    
    new_points = gaussians.get_xyz.shape[0]
    
    # 3. 保存新模型
    out_folder = f"output/pruned_scale_{scale_threshold}"
    save_path = os.path.join(out_folder, "point_cloud", "iteration_30000")
    os.makedirs(save_path, exist_ok=True)
    gaussians.save_ply(os.path.join(save_path, "point_cloud.ply"))
    
    # 拷贝配置文件
    if os.path.exists(os.path.join(base_model_path, "cfg_args")):
        shutil.copy(os.path.join(base_model_path, "cfg_args"), out_folder)
        
    print(f"尺度剪枝完成！阈值: {scale_threshold}")
    print(f"点数: {orig_points} -> {new_points} (缩减率: {(1 - new_points/orig_points)*100:.2f}%)")
    return new_points

if __name__ == "__main__":
    # 尺度通常很小，建议尝试 0.001 到 0.005 之间的值
    for s_t in [0.001, 0.002, 0.005]:
        prune_by_scale("output/truck_base", s_t)

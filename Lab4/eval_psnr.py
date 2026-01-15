import cv2
import os
import torch
import matplotlib.pyplot as plt
from skimage.metrics import peak_signal_noise_ratio as psnr
from skimage.metrics import structural_similarity as ssim

# --- 1. 实验数据配置 ---
# 请核对你的高斯点数量（从之前的 my_prune.py 输出中获得）
data = {
    "Baseline": {"count": 2056069, "path": "output/truck_base"},
    "0.01":     {"count": 1927877, "path": "output/pruned_0.01"},
    "0.05":     {"count": 1355999, "path": "output/pruned_0.05"},
    "0.1":      {"count": 1026830, "path": "output/pruned_0.1"},
    "0.2":      {"count": 700687,  "path": "output/pruned_0.2"}
}

# 比较第几张图片（通常选第一张 00001.png）
img_name = "00001.png"

def get_metrics(base_img_path, target_img_path):
    if not os.path.exists(target_img_path):
        return None, None
    
    img_base = cv2.imread(base_img_path)
    img_target = cv2.imread(target_img_path)
    
    # 统一尺寸
    img_target = cv2.resize(img_target, (img_base.shape[1], img_base.shape[0]))
    
    p = psnr(img_base, img_target)
    s = ssim(img_base, img_target, channel_axis=2)
    return p, s

# --- 2. 执行自动评估 ---
results = []
# 基准图路径 (以训练集渲染出的第一张图为准)
base_path = os.path.join(data["Baseline"]["path"], "train/ours_30000/renders", img_name)

print(f"{'Threshold':<10} | {'Count':<10} | {'PSNR':<8} | {'SSIM':<8}")
print("-" * 45)

for label, info in data.items():
    if label == "Baseline":
        results.append({"count": info["count"], "psnr": 50.0, "ssim": 1.0}) # 基准与自己比设为理想值
        print(f"{label:<10} | {info['count']:<10} | 100.00   | 1.0000")
        continue

    target_path = os.path.join(info["path"], "train/ours_30000/renders", img_name)
    p, s = get_metrics(base_path, target_path)
    
    if p:
        results.append({"count": info["count"], "psnr": p, "ssim": s})
        print(f"{label:<10} | {info['count']:<10} | {p:<8.2f} | {s:<8.4f}")

# --- 3. 绘制质量曲线图 ---
counts = [r["count"] for r in results]
psnrs = [r["psnr"] for r in results]
ssims = [r["ssim"] for r in results]

fig, ax1 = plt.subplots(figsize=(10, 6))

# 绘制 PSNR 曲线 (左轴)
color = 'tab:red'
ax1.set_xlabel('Number of Remaining Gaussians')
ax1.set_ylabel('PSNR (dB)', color=color)
ax1.plot(counts, psnrs, 'o-', color=color, linewidth=2, label='PSNR')
ax1.tick_params(axis='y', labelcolor=color)
ax1.invert_xaxis()  # 翻转横轴，体现点数减少过程

# 绘制 SSIM 曲线 (右轴)
ax2 = ax1.twinx()
color = 'tab:blue'
ax2.set_ylabel('SSIM', color=color)
ax2.plot(counts, ssims, 's--', color=color, linewidth=2, label='SSIM')
ax2.tick_params(axis='y', labelcolor=color)

plt.title('Quality Evaluation: Gaussian Count vs. PSNR/SSIM')
ax1.grid(True, linestyle='--', alpha=0.7)
fig.tight_layout()

# 保存图片
plt.savefig('quality_curve.png')
print("\n[成功] 质量曲线图已生成: quality_curve.png")

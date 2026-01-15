import matplotlib.pyplot as plt

# 填入你之前的实验数据
thresholds = [0, 0.01, 0.05, 0.1, 0.2]
counts = [2056069, 1927877, 1355999, 1026830, 700687]
fps = [2.83, 2.87, 2.92, 2.98, 3.04]  # it/s
sizes = [487, 456, 321, 243, 166]     # MB

# 设置中文字体（如果是英文系统建议用英文标签）
plt.style.use('seaborn-v0_8')

# 图 1: 剪枝阈值 vs. 剩余高斯数量
plt.figure(figsize=(10, 6))
plt.plot(thresholds, counts, 'r-o', label='Remaining Gaussians')
plt.xlabel('Opacity Threshold')
plt.ylabel('Count')
plt.title('Threshold vs. Remaining Gaussians')
plt.grid(True)
plt.savefig('threshold_vs_count.png')

# 图 2: 剩余高斯数量 vs. FPS (性能曲线)
plt.figure(figsize=(10, 6))
plt.plot(counts, fps, 'b-s', label='Rendering Speed (it/s)')
plt.xlabel('Number of Gaussians')
plt.ylabel('FPS (it/s)')
plt.gca().invert_xaxis() # 翻转横轴，展示点数减少、性能上升的过程
plt.title('Gaussian Count vs. Rendering Performance')
plt.grid(True)
plt.savefig('count_vs_fps.png')

# 图 3: 剩余高斯数量 vs. 模型大小 (压缩曲线)
plt.figure(figsize=(10, 6))
plt.plot(counts, sizes, 'g-^', label='Model Size (MB)')
plt.xlabel('Number of Gaussians')
plt.ylabel('Storage Size (MB)')
plt.gca().invert_xaxis()
plt.title('Gaussian Count vs. Model Size')
plt.grid(True)
plt.savefig('count_vs_size.png')

print("图表已生成：threshold_vs_count.png, count_vs_fps.png, count_vs_size.png")

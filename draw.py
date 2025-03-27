import matplotlib.pyplot as plt

def plot_graph(file_path):
    # 读取数据
    greedy_times = []
    improved_times = []

    with open(file_path, 'r') as f:
        lines = f.readlines()[1:]  # 跳过第一行（标题行）

    # 解析每一行的数据
    for line in lines:
        greedy, improved = map(int, line.strip().split(','))
        greedy_times.append(greedy)
        improved_times.append(improved)

    # 绘制折线图
    plt.plot(range(1, len(greedy_times) + 1), greedy_times, label="Greedy Algorithm", color="blue", marker="o")
    plt.plot(range(1, len(improved_times) + 1), improved_times, label="Improved Algorithm", color="red", marker="o")

    # 添加标题和标签
    plt.title("Greedy vs Improved Algorithm Total Time")
    plt.xlabel("Iterations")
    plt.ylabel("Total Time")
    plt.legend()

    # 显示图表
    plt.show()

# 添加main函数用于调试
if __name__ == "__main__":
    # 调用plot_graph并传入文件路径
    plot_graph("C:/Users/Litbug/Desktop/production_scheduling/scheduling_times.txt")

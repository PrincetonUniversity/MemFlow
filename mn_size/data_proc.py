import matplotlib.pyplot as plt

def process():
  f = open("mn_vs_mem.txt", 'r');
  mem = [];
  mb = [];
  nb = [];
  kb = [];
  spill = [];
  for line in f:
    info = [int(n) for n in line.split()];
    mem.append(info[0]);
    mb.append(info[1]);
    nb.append(info[2]);
    kb.append(info[3]);
    spill.append(info[4]);

  plt.figure();
  plt.plot(mem, mb, 'g-', label="mn_m", linewidth=2.0);
  plt.plot(mem, nb, 'r-', label="mn_n", linewidth=2.0);
  plt.plot(mem, kb, 'b-', label="mn_k", linewidth=2.0);
  plt.legend(loc='upper right');
  plt.title("Macro Node Size vs. Memory Size");
  plt.xlabel("Memory Size");
  plt.ylabel("Macro Node Size");
  plt.show();

  plt.figure();
  plt.plot(mem, spill, '-', linewidth=2.0);
  plt.title("#Spill vs. Memory Size");
  plt.xlabel("Memory Size");
  plt.ylabel("#Spill");
  plt.show();
  

if __name__=="__main__":
  process();

import numpy as np
import matplotlib.pyplot as plt

def main():
    BETA = 12.55 #bytes/cycle 47.68e9 B/s
    PI = 4 #flops/cycle 3.8e9 Hz
    RIDGE = PI/BETA
    RIDGE_SIMD = 4*RIDGE
    MIN, MAX = -4, 2
    I_a = np.array([2**MIN, RIDGE])
    I_b = np.array([RIDGE, RIDGE_SIMD])
    I_c = np.array([RIDGE_SIMD,2**MAX])

    scale = 1.75
    fig, ax = plt.subplots(1,figsize=(4*scale,3.5*scale))
    size=100

    ### a) ###
    ax.loglog(I_a,BETA*I_a, basex=2,basey=2, color='red')
    ax.loglog(I_b,BETA*I_b, basex=2,basey=2, linestyle='--',color='red')
    ax.hlines(PI,xmin=RIDGE,xmax=I_c[-1], color='red', label=r'Scalar $\pi$=4 flops/cycle'+'\n'+'$I_{Ridge}$=0.32 flops/byte')
    ax.hlines(PI*4,xmin=RIDGE_SIMD,xmax=I_c[-1], color='red',linestyle='--',label=r'SIMD $\pi$=16 flops/cycle'+'\n'+'$I_{Ridge}$=1.28 flops/byte')
    ax.set_xlabel("I(N) [flops / byte]")
    ax.set_ylabel("P(N) [flops / cycle]")

    D = 1
    euc = 3*D/8
    man = D/4
    pearson = 14*D/8
    cosine = 6*D/8

    ### Case (I) ###
    ax.scatter(euc,min(PI,euc*BETA), color='blue',marker='^',s=size)
    ax.scatter(man,min(PI,man*BETA),color='orange',marker='^',s=size)
    ax.scatter(pearson,min(PI,pearson*BETA),color='magenta',marker='^',s=size)
    ax.scatter(cosine,min(PI,cosine*BETA),color='lime',marker='^',s=size)

    ax.scatter(euc,min(4*PI,euc*BETA),color='blue',marker='*',s=size)
    ax.scatter(man,min(4*PI,man*BETA),color='orange',marker='*',s=size)
    ax.scatter(pearson,min(4*PI,pearson*BETA),color='magenta',marker='*',s=size)
    ax.scatter(cosine,min(4*PI,cosine*BETA),color='lime',marker='*',s=size)

    # only for legend
    ax.scatter(-1,min(4*PI,euc*BETA),label=r'L2', color='blue',marker='o',s=size)
    ax.scatter(-1,min(4*PI,man*BETA),label='L1 & Sup',color='orange',marker='o',s=size)
    ax.scatter(-1,min(4*PI,pearson*BETA),label='Pearson',color='magenta',marker='o',s=size)
    ax.scatter(-1,min(4*PI,cosine*BETA),label='Cosine',color='lime',marker='o',s=size)
    ax.scatter(-1,min(4*PI,cosine*BETA),label='Scalar',color='black',marker='^',s=size)
    ax.scatter(-1,min(4*PI,cosine*BETA),label='SIMD',color='black',marker='*',s=size)

    ax.set_xlim((2**MIN,2**MAX))
    ax.set_title(r"$N$ $\rightarrow$ $\infty$,  D = 1")
    # ax.set_title(r"$\beta$=12.55 byte/cycle")


    # ### c) ###
    # ax[1].scatter(3/16,min(3,3*BETA/16),label='comp1',color='blue',marker='*',s=size)
    # ax[1].scatter(3/16,min(2,3*BETA/16),label='comp2',color='orange',marker='*',s=size)
    # ax[1].scatter(3/20,min(2,3*BETA/20),label='comp3',color='magenta',marker='*',s=size)
    #
    # ax[1].scatter(3/16,min(4*3,3*BETA/16),label='comp1 vectorized',color='blue',marker='s',s=size+50)
    # ax[1].scatter(3/16,min(4*2,3*BETA/16),label='comp2 vectorized',color='orange',marker='s',s=size)
    # ax[1].scatter(3/20,min(4*2,3*BETA/20),label='comp3 vectorized',color='magenta',marker='s',s=size)

    ax.legend(loc="upper left")
    plt.tight_layout()
    # plt.show()
    plt.savefig("../roofline.eps",dpi=100)
    return 0

if __name__ == '__main__':
    main()

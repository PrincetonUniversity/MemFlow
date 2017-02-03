\ENCODING=ISO-8859-1
\Problem name: IloCplex

Minimize
 obj: 32 mb - 32 nb + x4 + [ 2 mb * nb + 2 mb * kb + 2 nb * kb ] / 2
Subject To
 c1: mb <= 2
 c2: nb <= 4
 c3: kb <= 32
Bounds
      mb >= 0
      nb >= 0
      kb >= 0
      x4 = -72
Generals
 mb  nb  kb 
End

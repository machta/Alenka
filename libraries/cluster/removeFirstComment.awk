BEGIN {
  doPrint = 0;
}

{
  if (doPrint == 1 || ! /\/\//) {
    print;
    doPrint = 1;
  }
}

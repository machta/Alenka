#
#    Copyright (C) 2016 Alois Schloegl <alois.schloegl@gmail.com>
#    This file is part of the "BioSig for C/C++" repository
#    (biosig4c++/libbiosig) at http://biosig.sf.net/
#
#    This program is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; either version 3
#    of the License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.


dyn.load("sload.so")

sload <- function(filename,channels=0) {
  result <- .Call("sload", filename, channels=0)
  return(result)
}


# Usage:
# x=sload("/home/as/data/test/cfs/BaseDemo/Leak.cfs");
# plot(x[,1])
# plot(x[,2])



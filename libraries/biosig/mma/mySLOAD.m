(*
  Platform independent loading of biosignal data
  Copyright 2011,2012,2016 Alois Schloegl, IST Austria
  This file is part of the "BioSig for C/C++" repository

BioSig is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation;either version 3
of the License,or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*)

mySLOAD[filename_, sweeps_] := Module [{link, data}, 
   (* mySLOAD: makes it platform independent *)
   
   link = Install[$SystemID<>"/sload.exe"];
   data = sload[filename, sweeps];
   Uninstall[link];
   data (* return data *)
   ];

(* This demonstrates the usage of the above module mySLOAD
On Windows platform the directory separators must be either 
a single forward slash "/" or double backward slashes "\\"
*)

filename = "AP100427b.dat";
data     = mySLOAD[filename, {1, 0, 0}];
signal   = data[[1]]   (* sampled data *)
t        = data[[2]]   (* sampling time points *)
hdrinfo  = data[[3]]   (* header information *)

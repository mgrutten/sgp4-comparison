# sgp4-comparison
Compare implementations of sgp4.

I am using the JSON version of the space-track catalogue, using the following steps (see [space-track](https://www.space-track.org/documentation#howto-api_curl))
```
$ curl -c cookies.txt -b cookies.txt https://www.space-track.org/ajaxauth/login -d 'identity=myusername&password=mY_S3cr3t_pA55w0rd!'
$ curl --cookie cookies.txt https://www.space-track.org/basicspacedata/query/class/gp/EPOCH/%3Enow-30/orderby/NORAD_CAT_ID,EPOCH/format/json > catalogue.json
```
The code that wraps each implementation
* reads the JSON catalogue file, then
* outputs a binary data file containing the propagated position and velocity for each object.

The binary data files have the form
* Number of objects - uint32
* Number of time-points per object - uint32
* For each object
  * NORAD ID - uint32
  * Time-point data (seconds since epoch, x, y, z, xv, yv, yz) - 7 double-precision floating point

The resulting data files are loaded and compared with a Matlab script.


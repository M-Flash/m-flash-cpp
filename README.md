# M-Flash: Fast billion-scale graph computation using a bimodal block processing model

## Dependencies
* [EasyLogging++](http://easylogging.muflihun.com). It is included in M-Flash
* [Boost](http://www.boost.org)
* [Eigen3](http://eigen.tuxfamily.org/). It is required for Lanczos Selective Orthogonalization

## Examples

### Pagerank 
```
cd [M-FLASH_ROOT];
make example_apps/pagerank
bin/example_apps/pagerank file [FILE_GRAPH] niters 4
```

## License
M-Flash is licensed under the Mozilla Public License, Version 2.0. 

#include <string>
#include <iostream>

#include <assert.h>
#include <stdlib.h>
#include <netcdf.h>
#include <time.h>

using namespace std;

#define CHECK_ERR(e) if(e != NC_NOERR){ cout << "Error: " << nc_strerror(e) << endl; exit(2); }


/*
 Hardcoded CDO Workflow 1
 select,october select,temp avg
 */

int main(int argc, char ** argv){
  if(argc != 2){
    cout << "Synopsis: " << argv[0] << " <input.nc>" << endl;
    exit(2);
  }

  char * file = argv[1];
  int ncid;
  int id_temp;
  int id_var_date;

  int x, y, ret;

  ret = nc_open(file, NC_NOWRITE, &ncid);
  CHECK_ERR(ret);

  int ndims, nvars, ngatts, unlimdimid;
  ret = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
  CHECK_ERR(ret);

  ret = nc_inq_varid(ncid, "snowcover", &id_temp);
  CHECK_ERR(ret);

  ret = nc_inq_varid(ncid, "time", &id_var_date);
  CHECK_ERR(ret);

  // find the dates for october
  int natts;
  nc_type datatype;
  ret = nc_inq_var(ncid, id_var_date, NULL, & datatype, &	ndims, NULL, &natts );
  CHECK_ERR(ret);
  assert(ndims == 1);
  assert(datatype == NC_DOUBLE);
  assert(ndims == 1);
  int id_dim_timestep;
  ret = nc_inq_var(ncid, id_var_date, NULL, NULL, NULL, &id_dim_timestep, NULL);
  CHECK_ERR(ret);

  size_t timesteps;
  ret = nc_inq_dim(ncid, id_dim_timestep, NULL, & timesteps);
  CHECK_ERR(ret);

  cout << "Timesteps: " << timesteps << endl;

  // now read the timesteps
  double * times = (double*) malloc(sizeof(double) * timesteps);
  ret = nc_get_var_double (ncid, id_var_date, times);
  CHECK_ERR(ret);

  // find october
  // convert: days 62,092 between 1/1/1800 and 1/1/1970
  int hours_1800 = 62091 * 24;

  for(size_t i = 0 ; i < timesteps; i++){
    long time = (times[i] - hours_1800) * 3600;
    struct tm date;
    gmtime_r(& time, & date);
    char buffer[200];
    strftime(buffer, 200, "%Y-%m-%d %H:%M:%S", & date);
    if(date.tm_mon == 9){ // october
      cout << buffer << endl;
    }
  }

  /* Close the file */
  ret = nc_close(ncid);
  CHECK_ERR(ret);

  cout << "Successful" << endl;
  return 0;
}

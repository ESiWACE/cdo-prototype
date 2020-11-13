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
  if(argc != 3){
    cout << "Synopsis: " << argv[0] << " <input.nc> <output.nc>" << endl;
    exit(2);
  }

  char * file = argv[1];
  char * outfile = argv[2];
  int ncid;
  int id_var_temp;
  int id_var_date;

  int x, y, ret;

  ret = nc_open(file, NC_NOWRITE, &ncid);
  CHECK_ERR(ret);

  int ndims, nvars, ngatts, unlimdimid;
  ret = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
  CHECK_ERR(ret);

  ret = nc_inq_varid(ncid, "snowcover", &id_var_temp);
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

  // the timestamps we need to process
  size_t * read_timestamps = (size_t*) malloc(sizeof(size_t) * timesteps);
  size_t t_index = 0;

  for(size_t i = 0 ; i < timesteps; i++){
    long time = (times[i] - hours_1800) * 3600;
    struct tm date;
    gmtime_r(& time, & date);
    if(date.tm_mon == 9){ // october
      char buffer[200];
      strftime(buffer, 200, "%Y-%m-%d %H:%M:%S", & date);
      cout << buffer << endl;
      read_timestamps[t_index++] = i;
    }
  }

  cout << "Will read " << t_index << " timestamps" << endl;



  // read the dimensions for the temp var
  ret = nc_inq_var(ncid, id_var_temp, NULL, & datatype, &	ndims, NULL, & natts );
  CHECK_ERR(ret);
  assert(ndims == 3); // 3d var
  // assert(datatype == NC_FLOAT);

  int id_dim_temp[3];
  ret = nc_inq_var(ncid, id_var_temp, NULL, NULL, NULL, id_dim_temp, NULL );
  CHECK_ERR(ret);

  size_t temp_size[3];
  ret = nc_inq_dim(ncid, id_dim_temp[0], NULL, & temp_size[0]);
  CHECK_ERR(ret);

  assert(id_dim_temp[0] == id_dim_timestep && "the first dimension must be time");

  ret = nc_inq_dim(ncid, id_dim_temp[1], NULL, & temp_size[1]);
  CHECK_ERR(ret);
  ret = nc_inq_dim(ncid, id_dim_temp[2], NULL, & temp_size[2]);
  CHECK_ERR(ret);

  cout << "Read for temp the dimensions: " << temp_size[0] << "x" << temp_size[1] << "x" << temp_size[2] << endl;

  // now create array of data
  size_t elems_2D = temp_size[1] * temp_size[2];
  size_t elems = temp_size[0] * temp_size[1] * temp_size[2];
  // data sum, and data read in this iteration
  float * d_sum = (float*) malloc(sizeof(float) * elems);
  float * d_read = (float*) malloc(sizeof(float) * elems);

  // COMPUTE
  // read the first timestamp
  size_t startp[3] = {0, 0, 0};
  size_t countp[3] = {1, temp_size[1], temp_size[2]};
  startp[0] = read_timestamps[0];
  ret = nc_get_vara_float(ncid, id_var_temp, startp, countp, d_sum);
  CHECK_ERR(ret);

  // actual computation
  for(int pos = 1; pos < t_index ; pos++){
    startp[0] = read_timestamps[pos];
    ret = nc_get_vara_float(ncid, id_var_temp, startp, countp, d_read);
    CHECK_ERR(ret);

    // do the maths:
    for(size_t x = 0; x < elems_2D; x++){
      d_sum[x] += d_read[x];
    }
  }

  // divide by the number of timesteps to compute average
  for(size_t x = 0; x < elems_2D; x++){
    d_sum[x] = d_sum[x] / t_index;
  }

  ret = nc_close(ncid);
  CHECK_ERR(ret);
  free(d_read);

  // output the result to a new NetCDF file
  ret = nc_create(outfile, NC_CLOBBER, &ncid);
  CHECK_ERR(ret);

  // define dimensions
  int dimidsp[2];
  int out_var_id;

  ret = nc_def_dim(ncid, "lon", temp_size[0], & dimidsp[0]);
  CHECK_ERR(ret);
  ret = nc_def_dim(ncid, "lat", temp_size[1], & dimidsp[1]);
  CHECK_ERR(ret);

  // define variable
  ret = nc_def_var	(ncid, "temp", NC_FLOAT, 2, dimidsp, & out_var_id);
  CHECK_ERR(ret);

  ret = nc_enddef(ncid);
  CHECK_ERR(ret);

  ret = nc_put_var_float (ncid, out_var_id, d_sum);
  CHECK_ERR(ret);

  // cleanup
  ret = nc_close(ncid);
  CHECK_ERR(ret);
  free(d_sum);

  cout << "Successful" << endl;
  return 0;
}

% Read an SGP4 comparison data file
function sat_data = read_data(fname)

% Open file
fid = fopen(fname, 'r');

% Number of entries and number of points per entry
num_entries = fread(fid, 1, 'uint32');
num_pts = fread(fid, 1, 'uint32');

for i=1:num_entries
    [norad_id, dt, teme] = read_record(fid, num_pts + 1);
    
    sat_data(i).norad_id = norad_id;
    sat_data(i).dt = dt;
    sat_data(i).teme = teme;
end

fclose(fid);



% Read a single record from a file
function [norad_id, dt, teme] = read_record(fid, num_pts)

norad_id = fread(fid, 1, 'uint32');

dt = nan(1, num_pts);
teme = nan(6, num_pts);

for i=1:num_pts
    dt(i) = fread(fid, 1, 'double');
    teme(:,i) = fread(fid, 6, 'double');
end

%% Load data from file and calculate errors
st = read_data('space-track\test.dat');
orekit = read_data('orekit\test.dat');
vallado = read_data('vallado\test.dat');

catalogue = jsondecode(fileread('catalogue.json'));


% Check the objects are the same
assert(length(st) == length(orekit), 'Different number of objects')
assert(all([st.norad_id] == [orekit.norad_id]), 'Mismatch in object sets')

assert(length(st) == length(vallado), 'Different number of objects')
assert(all([st.norad_id] == [vallado.norad_id]), 'Mismatch in object sets')

assert(length(st) == length(catalogue), 'Different number of objects')
assert(all([st.norad_id] == str2double({catalogue.NORAD_CAT_ID})), 'Mismatch in object sets')


% Calculate errors
orekit_pos_err = nan(1, length(st));
orekit_rel_err = nan(1, length(st));

vallado_pos_err = nan(1, length(st));
vallado_rel_err = nan(1, length(st));
for i=1:length(st)
    % orekit
    ref_pos = st(i).teme(1:3,:);
    dp = vecnorm(ref_pos - orekit(i).teme(1:3,:));

    orekit_pos_err(i) = max(dp);
    orekit_rel_err(i) = max(log10(dp) - log10(eps(vecnorm(ref_pos))));
    
    % vallado
    dp = vecnorm(ref_pos - vallado(i).teme(1:3,:));
 
    vallado_pos_err(i) = max(dp);
    vallado_rel_err(i) = max(log10(dp) - log10(eps(vecnorm(ref_pos))));

end

% Plot relative error
figure(1)
plot(1:length(st), orekit_rel_err, '.', 1:length(st), vallado_rel_err, '.')
xticks([])
xlim([1 length(st)])
ylabel('Digits of Position Error')
xlabel('Record Index')
legend('orekit', 'Vallado')

% Plot position error
figure(2)
sma = str2double({catalogue.SEMIMAJOR_AXIS});
loglog(sma, orekit_pos_err, '.', sma, vallado_pos_err, '.')
ylabel('Position Error (m)')
xlabel('Semi-Major Axis (km)')
legend('orekit', 'Vallado')


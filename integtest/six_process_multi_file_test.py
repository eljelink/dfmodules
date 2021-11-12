import pytest

import dfmodules.data_file_checks as data_file_checks
import integrationtest.log_file_checks as log_file_checks

# Values that help determine the running conditions
number_of_data_producers=5
number_of_readout_apps=3

# Default values for validation parameters
expected_number_of_data_files=5
check_for_logfile_errors=True
wib1_frag_hsi_trig_params={"fragment_type_description": "WIB", "hdf5_groups": "TPC/APA000",
                           "element_name_prefix": "Link", "element_number_offset": 0,
                           "expected_fragment_count": (number_of_data_producers*number_of_readout_apps),
                           "min_size_bytes": 37200, "max_size_bytes": 37200}
wib1_frag_multi_trig_params={"fragment_type_description": "WIB", "hdf5_groups": "TPC/APA000",
                             "element_name_prefix": "Link", "element_number_offset": 0,
                             "expected_fragment_count": (number_of_data_producers*number_of_readout_apps),
                             "min_size_bytes": 80, "max_size_bytes": 37200}
rawtp_frag_params={"fragment_type_description": "Raw TP", "hdf5_groups": "TPC/TP_APA000",
                   "element_name_prefix": "Link", "element_number_offset": (number_of_data_producers*number_of_readout_apps),
                   "expected_fragment_count": (number_of_data_producers*number_of_readout_apps),
                   "min_size_bytes": 80, "max_size_bytes": 80}
triggertp_frag_params={"fragment_type_description": "Trigger TP", "hdf5_groups": "Trigger/Region000",
                       "element_name_prefix": "Element", "element_number_offset": 0,
                       "expected_fragment_count": (number_of_data_producers*number_of_readout_apps),
                       "min_size_bytes": 80, "max_size_bytes": 80}

# The next three variable declarations *must* be present as globals in the test
# file. They're read by the "fixtures" in conftest.py to determine how
# to run the config generation and nanorc

# The name of the python module for the config generation
confgen_name="minidaqapp.nanorc.mdapp_multiru_gen"
# The arguments to pass to the config generator, excluding the json
# output directory (the test framework handles that)
confgen_arguments_base=[ "-d", "./frames.bin", "-o", ".", "-s", "10", "-n", str(number_of_data_producers), "-b", "1000", "-a", "1000", "-t", "10.0", "--max-file-size", "1074000000"] + [ "--host-ru", "localhost" ] * number_of_readout_apps
confgen_arguments={"WIB1_System": confgen_arguments_base,
                   "Software_TPG_System": confgen_arguments_base+["--enable-software-tpg"]}
# The commands to run in nanorc, as a list
nanorc_command_list="boot init conf start 101 resume wait 440 pause wait 2 stop wait 21 start 102 resume wait 330 pause wait 2 stop wait 21 scrap terminate".split()

# The tests themselves

def test_nanorc_success(run_nanorc):
    # Check that nanorc completed correctly
    assert run_nanorc.completed_process.returncode==0

def test_log_files(run_nanorc):
    if check_for_logfile_errors:
        # Check that there are no warnings or errors in the log files
        assert log_file_checks.logs_are_error_free(run_nanorc.log_files)

def test_data_file(run_nanorc):
    local_file_count=expected_number_of_data_files
    fragment_check_list=[]
    if "--enable-software-tpg" in run_nanorc.confgen_arguments:
        local_file_count=9
        fragment_check_list.append(wib1_frag_multi_trig_params)
        fragment_check_list.append(rawtp_frag_params)
        fragment_check_list.append(triggertp_frag_params)
    if len(fragment_check_list) == 0:
        fragment_check_list.append(wib1_frag_hsi_trig_params)

    # Run some tests on the output data file
    assert len(run_nanorc.data_files)==local_file_count

    for idx in range(len(run_nanorc.data_files)):
        data_file=data_file_checks.DataFile(run_nanorc.data_files[idx])
        assert data_file_checks.sanity_check(data_file)
        assert data_file_checks.check_file_attributes(data_file)
        for jdx in range(len(fragment_check_list)):
            assert data_file_checks.check_fragment_count(data_file, fragment_check_list[jdx])
            assert data_file_checks.check_fragment_presence(data_file, fragment_check_list[jdx])
            assert data_file_checks.check_fragment_size2(data_file, fragment_check_list[jdx])
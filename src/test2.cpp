#include <assert.h>
#include <iostream>
#include "env/composite_env_wrapper.h"
#include "options/options_helper.h"
#include "rocksdb/db.h"
#include "rocksdb/env.h"
#include "rocksdb/file_system.h"
#include "rocksdb/options.h"
#include "rocksdb/utilities/object_registry.h"
#include "rocksdb/utilities/options_util.h"
#include "options/options_helper.h"
#include "rocksdb/options.h"
static rocksdb::Env* fs_env = nullptr;
int main() {
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;
    std::shared_ptr<rocksdb::Env> fs_env_guard;
    rocksdb::ConfigOptions config_options(options);
    rocksdb::Env::CreateFromUri(config_options,"", "zenfs://dev:nvme0n1", &fs_env, &fs_env_guard);
    options.env = fs_env;
    options.use_direct_io_for_flush_and_compaction=true;
    // Open DataBase
    rocksdb::Status status = rocksdb::DB::Open(options, "rocksdbTest2", &db);
    if (status.ok()) {
	    std::cout << "Open DB" << std::endl;
    } else {
	    std::cerr << "Error: " << status.ToString() << std::endl; 
    }

    // Write
    status = db->Put(rocksdb::WriteOptions(), "my_name", "taerang");
    assert(status.ok());

    // Read
    std::string test_value;
    status = db->Get(rocksdb::ReadOptions(), "my_name", &test_value);
    if (status.ok()) {
        std::cout << test_value << std::endl;
    } else {
        std::cerr << "Error reading value" << std::endl;
    }

    // delete
    delete db;
}

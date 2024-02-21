#include <assert.h>
#include <iostream>
#include "rocksdb/db.h"

int main() {
    rocksdb::DB* db;
    rocksdb::Options options;
    options.create_if_missing = true;

    // 데이터베이스 열기
    rocksdb::Status status = rocksdb::DB::Open(options, "/tmp/testdb", &db);
    assert(status.ok());

    // 데이터 쓰기
    status = db->Put(rocksdb::WriteOptions(), "test", "value");
    assert(status.ok());

    // 데이터 읽기
    std::string test_value;
    status = db->Get(rocksdb::ReadOptions(), "test", &test_value);
    if (status.ok()) {
        std::cout << test_value << std::endl;
    } else {
        std::cerr << "Error reading value" << std::endl;
    }

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
	    std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
    }
    assert(it->status().ok());
    delete it;
    // 데이터베이스 닫기
    delete db;
}

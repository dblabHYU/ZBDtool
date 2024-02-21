#include <rocksdb/db.h>
#include <rocksdb/slice_transform.h>
#include <iostream>
#include <string>

using namespace rocksdb;

int main() {
    // 데이터베이스 및 옵션 설정
    DB* db;
    Options options;
    options.create_if_missing = true;
    options.prefix_extractor.reset(NewCappedPrefixTransform(10));
    options.comparator = BytewiseComparator();
    
    // 데이터베이스 열기
    Status s = DB::Open(options, "testprefix", &db);
    if (!s.ok()) {
        std::cerr << "데이터베이스 열기 실패: " << s.ToString() << std::endl;
        return 1;
    }

    // 데이터 삽입
    db->Put(WriteOptions(), "user1_name", "Alice");
    db->Put(WriteOptions(), "user1_age", "30");
    db->Put(WriteOptions(), "user2_name", "Bob");
    db->Put(WriteOptions(), "user2_age", "25");

    // 접두사 검색을 사용한 조회
    std::string prefix = "user";
    Iterator* it = db->NewIterator(ReadOptions());
    for (it->Seek(prefix); it->Valid() && it->key().starts_with(prefix); it->Next()) {
        std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
    }
    assert(it->status().ok());  // 검사
    delete it;

    // 데이터베이스 닫기
    delete db;
    return 0;
}


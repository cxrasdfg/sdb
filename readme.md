sdb
===

sdb是一个简单的RDBMS。

代码结构:
---------

代码现分为两个主要的layer，DB Layer实现数据库的储存引擎，SQL Layer包含SQL的解析|优化|执行．

### DB layer:

DB layer 已经实现表的增删查改和索引功能，已经支持foreign key．

1. src/db/io

    实现文件的io操作,包括增删读写文件，利用mmap实现的按块读写，配合索引提高随机读写效率．

2. src/db/record

    实现对记录的增删查改,支持可边长类型数据,但记录的长度不超过Block的长度.

3. src/db/bptree

    B+Tree的实现,支持针对主键增删查改.尚未重建索引和动态添加索引.

4. src/db/table

    表结构的实现,支持表的创建和删除,以及对记录的增删查改,支持谓词判断的查询.

5. src/db/cache

    缓冲器，实现了读写时间复杂度都为O(1)的LRU缓冲算法．

6. src/db/db

    数据库接口，提供增删查改功能,多表的外键参照完整性检查．

### SQL Layer

1.  SQL Parser(src/parser)

    手写的SQL递归下降语法分析器,暂时只支持少量DDL/DML,完整的实现需要等到DB Layer成熟之后开始实现.

2.  SQL Optimize

    SQL的优化,尚未实现.

3.  SQL Executor

    SQL执行器,尚未实现.

todo:
-----

### TODO DB Layer

- [X] IO
    - [X] read/write file block
    - [X] create/delete file

- [-] Record
    - [X] insert
    - [X] remove
    - [X] update
    - [ ] support big file

- [-] B+Bree
    - [X] insert
    - [X] write/read
    - [X] find
        - [X] range find
        - [X] value find
        - [X] mid find
        - [X] predicate find
    - [X] update
    - [X] print
    - [X] remove
    - [ ] build
    - [ ] rebuild

- [X] Table
    - [X] find
        - [X] value find
        - [X] predicate find
    - [X] remvore
        - [X] value remove
        - [X] predicate remove
    - [X] insert
    - [X] update

- [X] LRU Cache
    - [X] get/put
    - [X] write/read block
    - [  ] write/read file cache

- [X] db
    - [X] find
    - [X] remove
    - [X] insert

- [  ] Test

- [  ] Transaction support

### TODO SQL Layer

1.  TODO SQL Parser
    - [X] DDL
        - [X] create table
    - [-] DML
        - [X] query
        - [ ] insert column

2.  TODO SQL 优化器

3.  TODO SQL 执行器



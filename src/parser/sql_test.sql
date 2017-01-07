-- test
/*
create table T(
    id int not null,
    name varchar(10),
    primary key (id),
    foreign (id) references T2
);
*/
select id, id2, id3
from good1, good2;

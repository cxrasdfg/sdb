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
from good1, good2
where id = 1 and id = 2 or id2 = 1 and id2 = 3;

create table WIZ_META
(
   META_NAME                       varchar(50),
   META_KEY                        varchar(50),
   META_VALUE                      varchar(3000),
   DT_MODIFIED                     char(19),
   primary key (META_NAME, META_KEY)
)

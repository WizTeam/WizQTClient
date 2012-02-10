create table WIZ_DOCUMENT_PARAM
(
   DOCUMENT_GUID                  char(36)                       not null,
   PARAM_NAME                     varchar(50)                    not null,
   PARAM_VALUE                    varchar(3000),
   primary key (DOCUMENT_GUID, PARAM_NAME)
)
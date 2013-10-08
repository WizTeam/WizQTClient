create table WIZ_STYLE
(
   STYLE_GUID                       char(36)                       not null,
   STYLE_NAME                       varchar(150),
   STYLE_DESCRIPTION                varchar(600),
   STYLE_TEXT_COLOR                 char(6),
   STYLE_BACK_COLOR                 char(6),
   STYLE_TEXT_BOLD                  int,
   STYLE_FLAG_INDEX                 int,
   DT_MODIFIED                      char(19),
   WIZ_VERSION                      int64,
   primary key (STYLE_GUID)
)
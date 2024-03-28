USE zenconn;

create table servers (
    uuid char(36) not null primary key,
    shutdown_at timestamp null,
    created_at timestamp null,
    updated_at timestamp null
) collate = utf8mb4_unicode_ci;

create table sessions (
    uuid char(36) not null primary key,
    disconnected_at timestamp null,
    created_at timestamp null,
    updated_at timestamp null
) collate = utf8mb4_unicode_ci;
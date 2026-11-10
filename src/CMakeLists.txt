set(_SOURCE
    main.cxx
    server.cxx
    parse_dns.cxx
    FileRecord.cxx
)

set(_CLIENT
    client.cxx
)

add_executable(dnsm ${_SOURCE})
target_include_directories(dnsm PUBLIC ${INCLUDE_DIR})
target_link_libraries(dnsm
    Ws2_32
)


add_executable(client ${_CLIENT})
target_include_directories(client PUBLIC ${INCLUDE_DIR})
target_link_libraries(client
    Ws2_32
)
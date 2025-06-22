
typedef enum {
    FALSE = 0,
    TRUE = 1
} Boolean;

typedef enum {
    PARSE_INCOMPLETE = 0,  // 아직 패킷 미완성
    PARSE_COMPLETE   = 1,  // 패킷 완성
    PARSE_ERROR      = -1  // 패킷 에러 (길이 이상 등)
} ParseResult;
1조 (김유민, 이우진, 정아윤, 허서연) Code Report

1. 메인 구동 파일
- GameObject: 기본 게임 객체 class, 순수 가상 함수인 virtual void update() = 0 이용하여 추상 클래스로 정의함.
- Player: 플레이어의 상태(좌표, 골드, 스태미나, 경험치 등) 및 스킬 트리(Skill Tree) 노드 상태 저장. GameObject 상속받아 update()를 override함으로써 플레이어 상태 갱신
- Game: 메인 게임 루프, 유저 입력(스킬창 팝업 등) 및 서브 시스템 출력 제어
- GuiRenderer / Renderer: SDL2 화면 출력, 윈도우 생성 및 렌더링에 메모리 포인터 사용(SDL_Window*, SDL_Texture*)
- Map: 2차원 타일맵 정보를 관리, 맵 관리에 std::vector 사용

2. 게임 시스템 구현
- CropSystem / LivestockSystem: 작물 심기/수확 및 가축 생산 클래스
- MarketSystem: 작물 판매, 씨앗 구매 등 상점 관리 클래스
- WorkshopSystem / KitchenSystem: 도구 업그레이드 및 요리 시스템, std::vector 및 std::map을 이용해 레시피 및 레시피별 재료 저장
- LevelSystem: 경험치 누적 시 스킬 포인트 지급 및 레벨업 제어 클래스
- EventSystem: 가뭄, 한파 등 무작위 확률 이벤트 발생 클래스, std::vector 이용해 이벤트 팝업 메시지 관리

3. 플레이어 데이터 관리
- Inventory: 템플릿 이용하여 아이템 소지량 제어, std::map 이용하여 아이템별 개수 검색 및 수정
- Position: (x, y) 좌표 저장, 좌표 간 덧셈 및 일치 여부 판별 위해 operator+, operator== 연산자 오버로딩 사용, 좌표 출력에 friend operator<< 사용
- Tile, ItemType: 타일 속성, 아이템 식별자 정의
- Tutorial, MiniGames: 튜토리얼 팝업 및 미니게임 로직 클래스 (광산, 낚시)

4. 생성형 AI 사용
- 활용 목적: SDL2 렌더링 초기 뼈대 코드 작성 및 아이템 데이터 switch문 작성
- 오류 수정: 렌더링 시 문제를 포인터 해제 코드(SDL_DestroyTexture) 수동 추가를 통해 해결함

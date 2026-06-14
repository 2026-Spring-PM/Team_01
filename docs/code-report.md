# 코드 보고서 (Code Report)

## a) 전반적인 모듈 구조 및 클래스 설계
본 프로젝트는 중앙 컨트롤러인 `Game` 클래스를 중심으로 각 기능이 철저히 객체 지향적으로 분리되도록 구성되었습니다:
- **`Game` 클래스 (`Game.h`)**: `Player`, `Map`, `Renderer`, 및 다수의 `*System` 인스턴스들을 캡슐화하여 멤버 변수로 가집니다. `run()` 메서드를 통한 메인 루프를 돌며, 사용자 입력(`handle_input()`)에 따라 각 서브 시스템에 명령을 내리는 코어 역할을 합니다.
- **`Player` 클래스 (`Player.h`)**: 최상위 객체인 `GameObject`를 상속받습니다. 위치 좌표(x, y), 재화(money, stamina), 인벤토리(`Inventory`), 경험치(`LevelSystem`), 스킬트리 노드 배열 등을 캡슐화하고 있으며, `try_move`, `invest_str_node` 등 플레이어 행위와 관련된 책임을 독점합니다.
- **서브 시스템 클래스**: 
  - `CropSystem`, `BarnSystem` 등은 도메인별 기능의 집합으로, `Tile`과 `Player` 객체를 매개변수로 받아 상태를 변경하는 정적(static) 메서드나 싱글톤에 준하는 형태로 구현되었습니다.
  - `EventSystem`은 상태를 가져야 하므로 클래스 형태로 존재하며, 랜덤 이벤트를 계산하여 `EventDayReport` 구조체를 반환합니다.

## b) 시스템 컴포넌트 주요 로직 설명
- **농사 및 상호작용 (`CropSystem` 등)**: 사용자 입력이 발생하면 `Game::handle_farm()`에서 `CropSystem::plant_crop`이나 `harvest_crop`을 호출합니다. 이때 참조(`&`)로 넘겨진 `Tile`의 상태(씨앗 유무, 수분 상태)와 `Player`의 `Inventory`에 있는 씨앗 개수가 증감되도록 연동되어 있습니다.
- **이벤트 시스템 (`EventSystem`)**: 하루가 넘어갈 때(`advance_day`) 호출되는 `process_new_day()` 함수를 통해 내부 확률(`drought_chance_` 등)에 따라 가뭄, 한파, 야생동물 습격 여부를 결정합니다. 결과는 구조체에 담겨 UI 시스템으로 넘어가 팝업 메시지로 띄워집니다.
- **UI 및 렌더링 (`GuiRenderer`)**: `FARM_GUI` 매크로를 통해 분기되며, 내부적으로 SDL2의 창(`SDL_Window`)과 렌더러(`SDL_Renderer`) 포인터를 관리합니다. 맵의 타일 좌표를 화면 픽셀 좌표로 환산해 이미지를 오버레이 렌더링하는 구조로 되어 있습니다.

## c) C++ 주요 개념 활용 영역
- **객체 지향 프로그래밍 (OOP) 및 상속**: `GameObject` 기본 클래스를 상속받아 `Player`를 구현하는 등 계층 구조와 다형성을 활용했습니다.
- **캡슐화 (Encapsulation)**: 멤버 변수를 `private`으로 묶고, 데이터를 조작할 때만 `spend_money()`나 `use_stamina()` 같은 특정 인터페이스로 접근하도록 하여 게임 밸런스 데이터 오염을 방지했습니다.
- **STL 및 구조체**: `EventDayReport`나 `CropInfo` 같은 구조체를 적극 활용해 데이터 묶음을 반환하며, `std::vector`(이벤트 팝업 메시지 배열 등), `std::string`, `std::map` 컨테이너를 사용하여 동적 배열과 문자열 관리를 수행했습니다.

## d) 생성형 AI 활용 (※ 실제 내용으로 수정 필요)
- **사용 목적**: SDL2 그래픽 라이브러리 초기 연동을 위한 보일러플레이트(Boilerplate) 뼈대 코드 생성 및, 방대한 `CropInfo` 기초 데이터 스위치문 작성을 위해 ChatGPT를 사용했습니다.
- **코드 변경점**: `EventSystem`의 로직과 화면 출력부가 엉켜 있던 초기 코드를 개선하고자 AI에 리팩토링을 요청했고, 제안받은 `EventDayReport` 구조체 반환 방식을 채택해 로직과 UI를 깔끔하게 분리했습니다.
- **오류 식별 및 수정**: AI가 작성한 `GuiRenderer` 부분에서 프레임 저하와 텍스처 메모리 릭(Leak)이 발생했습니다. 오류 로그 및 메모리 사용량 프로파일링 결과를 AI에게 역으로 입력해 `SDL_DestroyTexture`가 누락된 루프를 찾아내고 해제 코드를 추가하여 해결했습니다.

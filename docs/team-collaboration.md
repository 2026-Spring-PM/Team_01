1조 (김유민, 정아윤, 이우진, 허서연) Team Collaboration

1. 팀원별 기여 내용
- 김유민: 프로젝트 초기 코어 구조(Map, Player, Level, Inventory, Renderer) 설계 및 구현, GUI Docker 이미지 환경을 구성하고, 빌드 에러 수정
- 정아윤: 괴식 시스템 및 이벤트 시스템 구현, 주방, 제작소, 스태미나 시스템 UI 개선, 건설 모듈 기본 코드 작성
- 이우진: 프로젝트 초기 디렉토리 구조 셋업, GUI 환경 전반적으로 구현 및 수정
- 허서연: 상점 시스템 및 주방, 미니게임 등 구현, macOS 환경 GUI 렌더링 에러 수정, 튜토리얼 시스템 추가

2. 개인별 Top 3 Git commit
- 김유민:
  1. b405c0d: feat(core): 프로젝트의 핵심 뼈대인 Map, Player, Level, Inventory, Renderer를 초기 구현
  2. e201be3: GUI Docker 이미지 적용으로 팀원 간 개발 환경 통일
  3. 5049198: merge: fix-build into demo_02 (빌드 에러 수정 후 데모 브랜치 병합)
- 정아윤:
  1. 455d1da: 괴식 시스템 전까지 구현 (새로운 콘텐츠 추가)
  2. 58f26ee: 스태미나 시스템, 주방, 제작소 UI 전면 수정
  3. f9cb1ba: 이벤트 시스템 수정 및 스킬창 UI 개선
- 이우진:
  1. f4e933a: GUI 환경 및 2x2 건물 구조 변경, 축사 시스템 추가
  2. e67a860: GUI 인벤토리 저장소 및 잠금 UI 개선
  3. 8b562ec: 프로젝트 초기 src 디렉토리 생성 및 구조 셋업
- 허서연:
  1. e5871af: 튜토리얼 추가 및 macOS 화면 깨짐 수정
  2. 08d37bc: 작물 성장 현황 표시, 레벨업 문구 추가, Market 계절별 가격 변동 시스템 추가
  3. cd1514d: GUI 화면 깜빡임 수정, 주방 수량 버그 수정, 스태미나 비용 조정

3. Git Graph 및 주요 커밋 분석

*   5049198 - 김유민: merge: fix-build into demo_02
*   419c9f9 - 허서연: Merge pull request #2 from 2026-Spring-PM/Market
|\  
| | * fa67c48 - 정아윤: 괴식 관련 로직 및 UI 사소한 수정
| | * f9cb1ba - 정아윤: 이벤트 시스템 수정, 스킬창 UI 개선
| | * 2831b0f - 이우진: Add files via upload (GUI 코드 기여)
| | * 85aa0f1 - 허서연: Resolve merge conflicts and apply remote changes
| | *   1ae3adc - 허서연: Merge remote-tracking branch 'origin/demo_02' into demo_02
| | * b73ed33 - 허서연: Fix macOS font path for GuiRenderer
| * e201be3 - 김유민: GUI Docker 이미지 적용
* b405c0d - 김유민: feat(core): map, player, level, inventory, render

- b405c0d (김유민): 프로젝트 초기 코어 구조(Map, Player, LevelSystem, Inventory, Renderer) 설계 및 구현
- e201be3, 5049198 (김유민): GUI Docker 이미지를 도입, 빌드 에러 수정 후 demo_02 브랜치에 병합
- f9cb1ba, fa67c48 (정아윤): 이벤트 시스템 로직 수정, 스킬창 UI 및 괴식 관련 UI 수정
- 2831b0f (이우진): demo_02 브랜치에 GUI 관련 코드 업로드
- 1ae3adc, 85aa0f1 (허서연): 동시 작업으로 발생한 Merge Conflict를 원격 브랜치를 이용해 해결 및 병합
- b73ed33 (허서연): macOS 환경에서 폰트 경로 미인식으로 인한 오류 해결
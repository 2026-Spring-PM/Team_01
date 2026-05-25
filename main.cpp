#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <unistd.h>
#include <limits> 

#include "ConfigManager.hpp"
#include "Player.hpp"
#include "FarmMap.hpp"
#include "ShopManager.hpp"
#include "Kitchen.hpp"
#include "Workshop.hpp"
#include "MiniGames.hpp"
#include "EventManager.hpp"
#include "Crop.hpp"
#include "Livestock.hpp"

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0) perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) perror ("tcsetattr ~ICANON");
    return buf;
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));
    ConfigManager::getInstance(); 

    Player player;
    FarmMap map;
    ShopManager shop;
    Kitchen kitchen;
    Workshop workshop;
    FishingGame fishing;
    LottoGame lotto;
    EventManager events;

    player.addItem("구리 광석", 2);
    player.addItem("철 광석", 1);

    int currentDay = 1;
    bool isRunning = true;

    while (isRunning && currentDay <= 100) {
        shop.startNewDay();
        bool isSummer = (currentDay > 25 && currentDay <= 50);
        events.triggerEvents(player, isSummer);

        bool dayActive = true;
        std::string systemMessage = "새로운 하루가 시작되었습니다!"; 

        while (dayActive && isRunning) {
            system("clear"); 
            std::cout << "========================================\n";
            std::cout << "☀️ [Day " << currentDay << "] " << systemMessage << "\n";
            std::cout << "========================================\n";

            map.drawMap(player.getX(), player.getY());
            
            std::cout << "\n▶ 스태미나: " << player.getStamina() << "/" << player.getMaxStamina() 
                      << " | 골드: " << player.getGold() << "G | 레벨: " << player.getLevel() << "\n";
            std::cout << "========================================\n";
            std::cout << " [W/A/S/D] 이동 | [J] 상호작용 | [E] 가방 확인 | [I] 구매 | [U] 판매 | [5] 잠자기\n";
            std::cout << "========================================\n";

            systemMessage = ""; 
            char key = getch();
            
            if (key == 'w' || key == 'W') {
                if (map.isWalkable(player.getX(), player.getY() - 1)) player.setPos(player.getX(), player.getY() - 1);
            } else if (key == 's' || key == 'S') {
                if (map.isWalkable(player.getX(), player.getY() + 1)) player.setPos(player.getX(), player.getY() + 1);
            } else if (key == 'a' || key == 'A') {
                if (map.isWalkable(player.getX() - 1, player.getY())) player.setPos(player.getX() - 1, player.getY());
            } else if (key == 'd' || key == 'D') {
                if (map.isWalkable(player.getX() + 1, player.getY())) player.setPos(player.getX() + 1, player.getY());
            }
            
            else if (key == 'e' || key == 'E') {
                system("clear");
                std::cout << "\n==========================================================\n";
                std::cout << "🎒 [내 가방(인벤토리) 실시간 현황]\n";
                std::cout << "==========================================================\n";
                std::cout << " 🌱 씨앗/자재 자산:\n";
                std::cout << "  - 감자 씨앗: " << player.getItemCount("감자 씨앗") << "개\n";
                std::cout << "  - 당근 씨앗: " << player.getItemCount("당근 씨앗") << "개\n";
                std::cout << "  - 밀 씨앗:   " << player.getItemCount("밀 씨앗") << "개\n";
                std::cout << "  - 쌀 씨앗:   " << player.getItemCount("쌀 씨앗") << "개\n";
                std::cout << "  - 사과 묘목: " << player.getItemCount("사과 묘목") << "개\n";
                std::cout << "  - 나무 자재: " << player.getItemCount("나무 (건축자재)") << "개\n";
                std::cout << " 💎 광석 재료 자산:\n"; 
                std::cout << "  - 구리 광석: " << player.getItemCount("구리 광석") << "개\n";
                std::cout << "  - 철 광석:   " << player.getItemCount("철 광석") << "개\n";
                std::cout << "  - 금 광석:   " << player.getItemCount("금 광석") << "개\n";
                std::cout << " 🐾 가축 구매 보유목록:\n";
                std::cout << "  - 새끼 닭:   " << player.getItemCount("새끼 닭") << "개\n";
                std::cout << "  - 새끼 돼지: " << player.getItemCount("새끼 돼지") << "개\n";
                std::cout << "  - 새끼 소:   " << player.getItemCount("새끼 소") << "개\n";
                std::cout << " 🌾 현재 수확물 및 가공 식자재 창고:\n";
                bool hasStock = false;
                std::vector<std::string> targetList = {"감자", "당근", "밀", "쌀", "사과", "달걀", "우유", "돼지고기", "고등어", "광어", "참치", "빵", "사과파이", "생선구이"};
                for (const auto& name : targetList) {
                    if (player.getItemCount(name) > 0) {
                        std::cout << "  - [" << name << "]: " << player.getItemCount(name) << "개 보유 중\n";
                        hasStock = true;
                    }
                }
                if (!hasStock) std::cout << "  (현재 보관함에 보관 중인 수확 및 생산물이 전무합니다.)\n";
                std::cout << "==========================================================\n";
                std::cout << "[아무 키나 누르면 농장 화면으로 복귀합니다]" << std::flush;
                getch();
                systemMessage = "인벤토리를 확인하고 돌아왔습니다.";
            }

            else if (key == 'j' || key == 'J') {
                int px = player.getX();
                int py = player.getY();
                Tile& currentTile = map.getTile(px, py);

                if (px == 11 && py == 11) {
                    system("clear");
                    std::cout << "\n💎 광산 입구입니다. 로또 게임을 진행하시겠습니까? (Y/N): " << std::flush;
                    char ans = getch();
                    if (ans == 'y' || ans == 'Y') {
                        system("clear");
                        lotto.playLotto(player);
                        std::cout << "\n[아무 키나 눌러 돌아가기]" << std::flush; getch();
                    }
                    continue;
                }

                if (currentTile.type == TileType::Riverbank) {
                    system("clear");
                    std::cout << "\n🎣 강변 모래사장입니다. 낚시를 시작하시겠습니까? (Y/N): " << std::flush;
                    char ans = getch();
                    if (ans == 'y' || ans == 'Y') {
                        system("clear");
                        fishing.playFishing(player);
                        std::cout << "\n[아무 키나 눌러 돌아가기]" << std::flush; getch();
                    }
                }
                
                else if (currentTile.type == TileType::Livestock) {
                    if (!currentTile.isOccupied) {
                        system("clear");
                        std::cout << "\n🐾 [사육 공간] 키울 가축의 번호를 선택하세요.\n";
                        std::cout << "1. 새끼 닭 (보유: " << player.getItemCount("새끼 닭") << ")\n";
                        std::cout << "2. 새끼 돼지 (보유: " << player.getItemCount("새끼 돼지") << ")\n";
                        std::cout << "3. 새끼 소 (보유: " << player.getItemCount("새끼 소") << ")\n";
                        std::cout << "0. 취소\n>> " << std::flush;
                        char sel = getch();
                        
                        std::string targetAnimal = "";
                        Livestock* newAnimal = nullptr;
                        if (sel == '1') { targetAnimal = "새끼 닭"; newAnimal = new Chicken(); }
                        else if (sel == '2') { targetAnimal = "새끼 돼지"; newAnimal = new Pig(); }
                        else if (sel == '3') { targetAnimal = "새끼 소"; newAnimal = new Cow(); }

                        if (newAnimal != nullptr) {
                            if (player.getItemCount(targetAnimal) > 0) {
                                player.removeItem(targetAnimal, 1);
                                currentTile.animalPtr = newAnimal;
                                currentTile.isOccupied = true;
                                systemMessage = targetAnimal + "을(를) 농장에 배치했습니다!";
                            } else {
                                delete newAnimal;
                                systemMessage = "해당 가축이 인벤토리에 없습니다!";
                            }
                        }
                    } else {
                        // 🌟 개선됨: 상호작용 및 제거 선택 메뉴 추가
                        system("clear");
                        std::cout << "\n🐄 [" << currentTile.animalPtr->getName() << "] 무엇을 하시겠습니까?\n";
                        std::cout << "1. 상호작용 (생산물 수집)\n";
                        std::cout << "2. 가축 제거 (놓아주기/도축)\n";
                        std::cout << "0. 취소\n>> " << std::flush;
                        char ans = getch();
                        
                        if (ans == '1') {
                            if (currentTile.animalPtr->getHasProducedToday()) {
                                systemMessage = "❌ 오늘은 이미 상호작용을 마쳤습니다. 내일 다시 와주세요!";
                            } else if (!currentTile.animalPtr->getIsAdult()) {
                                systemMessage = "🐣 아직 다 자라지 않아 생산품을 얻을 수 없습니다.";
                            } else {
                                auto result = currentTile.animalPtr->produce(player);
                                if (result.second > 0) {
                                    std::string animalProduct = "";
                                    if (result.first == ProductType::Egg) animalProduct = "달걀";
                                    else if (result.first == ProductType::Pork) animalProduct = "돼지고기";
                                    else if (result.first == ProductType::Milk) animalProduct = "우유";
                                    
                                    if (animalProduct != "") player.addItem(animalProduct, result.second);
                                    systemMessage = currentTile.animalPtr->getName() + " 생산품(" + animalProduct + ") " + std::to_string(result.second) + "개 수집 완료!";
                                    
                                    // 돼지 등 도축되는 가축 처리
                                    if (currentTile.animalPtr->getIsSlaughtered()) {
                                        delete currentTile.animalPtr;
                                        currentTile.animalPtr = nullptr;
                                        currentTile.isOccupied = false;
                                    }
                                } else {
                                    systemMessage = "체력이 부족하여 상호작용할 수 없습니다.";
                                }
                            }
                        } else if (ans == '2') {
                            std::cout << "\n정말로 이 가축을 농장에서 제거하시겠습니까? (Y/N): " << std::flush;
                            char confirm = getch();
                            if (confirm == 'y' || confirm == 'Y') {
                                delete currentTile.animalPtr;
                                currentTile.animalPtr = nullptr;
                                currentTile.isOccupied = false;
                                systemMessage = "가축을 농장에서 성공적으로 제거했습니다.";
                            }
                        }
                    }
                }

                else if (currentTile.type == TileType::Agriculture) {
                    if (!currentTile.isOccupied) {
                        system("clear");
                        std::cout << "\n🌱 [작물 공간] 심을 씨앗의 번호를 선택하세요.\n";
                        std::cout << "1. 감자 씨앗 (보유: " << player.getItemCount("감자 씨앗") << ")\n";
                        std::cout << "2. 당근 씨앗 (보유: " << player.getItemCount("당근 씨앗") << ")\n";
                        std::cout << "3. 밀 씨앗 (보유: " << player.getItemCount("밀 씨앗") << ")\n";
                        std::cout << "4. 쌀 씨앗 (보유: " << player.getItemCount("쌀 씨앗") << ")\n";
                        std::cout << "5. 사과 묘목 (보유: " << player.getItemCount("사과 묘목") << ")\n";
                        std::cout << "0. 취소\n>> " << std::flush;
                        char sel = getch();
                        
                        std::string targetSeed = "";
                        Crop* newCrop = nullptr;
                        if (sel == '1') { targetSeed = "감자 씨앗"; newCrop = new Potato(); }
                        else if (sel == '2') { targetSeed = "당근 씨앗"; newCrop = new Carrot(); }
                        else if (sel == '3') { targetSeed = "밀 씨앗"; newCrop = new Wheat(); }
                        else if (sel == '4') { targetSeed = "쌀 씨앗"; newCrop = new Rice(); }
                        else if (sel == '5') { targetSeed = "사과 묘목"; newCrop = new AppleTree(); }

                        if (newCrop != nullptr) {
                            if (player.getItemCount(targetSeed) > 0) {
                                if (player.consumeStamina(10)) {
                                    player.removeItem(targetSeed, 1);
                                    currentTile.cropPtr = newCrop;
                                    currentTile.isOccupied = true;
                                    systemMessage = targetSeed + "을(를) 심었습니다! (체력 -10)";
                                } else {
                                    delete newCrop;
                                    systemMessage = "❌ 스태미나가 부족하여 씨앗을 심을 수 없습니다!";
                                }
                            } else {
                                delete newCrop;
                                systemMessage = "해당 씨앗이 인벤토리에 없습니다!";
                            }
                        }
                    } else {
                        system("clear");
                        std::cout << "\n🌾 심어진 작물: " << currentTile.cropPtr->getName() << " | 수확하시겠습니까? (Y/N): " << std::flush;
                        char ans = getch();
                        if (ans == 'y' || ans == 'Y') {
                            system("clear");
                            int yield = currentTile.cropPtr->harvest(player);
                            if (yield > 0) {
                                player.addItem(currentTile.cropPtr->getName(), yield);
                                systemMessage = currentTile.cropPtr->getName() + " " + std::to_string(yield) + "개 수확 완료!";
                                if (!currentTile.cropPtr->getIsPerennial()) {
                                    delete currentTile.cropPtr;
                                    currentTile.cropPtr = nullptr;
                                    currentTile.isOccupied = false;
                                }
                            } else {
                                systemMessage = "아직 다 자라지 않았거나 체력이 부족합니다.";
                            }
                            std::cout << "\n[아무 키나 눌러 돌아가기]" << std::flush; getch();
                        }
                    }
                }

                else if (currentTile.type == TileType::Construction) {
                    if (!currentTile.isOccupied) {
                        system("clear");
                        std::cout << "\n🔨 [건설 공간] 지을 건물의 번호를 선택하세요.\n";
                        std::cout << "1. 주방 (요리 제작)\n";
                        std::cout << "2. 작업장 (농기구 업그레이드)\n";
                        std::cout << "0. 취소\n>> " << std::flush;
                        char sel = getch();
                        if (sel == '1') {
                            currentTile.buildingName = "주방";
                            currentTile.isOccupied = true;
                            systemMessage = "주방을 성공적으로 건설했습니다!";
                        } else if (sel == '2') {
                            currentTile.buildingName = "작업장";
                            currentTile.isOccupied = true;
                            systemMessage = "작업장을 성공적으로 건설했습니다!";
                        }
                    } else {
                        if (currentTile.buildingName == "주방") {
                            kitchen.interact(player);
                            systemMessage = "주방에서 요리 일과를 마쳤습니다.";
                        } else if (currentTile.buildingName == "작업장") {
                            workshop.interact(player);
                            systemMessage = "작업장 볼일을 마쳤습니다.";
                        }
                    }
                }
            }

            else if (key == 'i' || key == 'I') {
                bool inShop = true;
                while (inShop) {
                    system("clear");
                    shop.showBuyMenu(player);
                    std::cout << "\n[구매] 번호 입력 후 엔터 (구매번호: 0~" << shop.getBuyItemCount() - 1 << " | 나가기: 99)\n>> ";
                    
                    int shopChoice;
                    std::cin >> shopChoice;

                    if (std::cin.fail()) {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cout << "\n⚠️ 잘못된 입력 형태입니다. 숫자를 입력하세요.\n[아무 숫자나 입력 후 엔터]";
                        int d; std::cin >> d; std::cin.clear(); std::cin.ignore();
                        continue;
                    }

                    if (shopChoice == 99) {
                        inShop = false;
                    } 
                    else if (shopChoice >= 0 && shopChoice < shop.getBuyItemCount()) {
                        std::cout << "구매할 수량을 입력하세요: ";
                        int qty; std::cin >> qty;
                        
                        if (std::cin.fail() || qty <= 0) {
                            std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                            std::cout << "❌ 잘못된 수량입니다.\n[엔터키 누르기]" << std::flush; getch();
                            continue;
                        }
                        shop.buyFromPierre(player, shopChoice, qty);
                        std::cout << "\n[아무 숫자나 입력 후 엔터]"; int dummy; std::cin >> dummy;
                    }
                }
                systemMessage = "물건 구매를 마쳤습니다.";
            }

            else if (key == 'u' || key == 'U') {
                bool inShop = true;
                while (inShop) {
                    system("clear");
                    shop.showSellMenu(player);
                    std::cout << "\n[판매] 번호 입력 후 엔터 (판매번호: 0~" << shop.getSellItemCount() - 1 << " | 나가기: 99)\n>> ";
                    
                    int shopChoice;
                    std::cin >> shopChoice;

                    if (std::cin.fail()) {
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        std::cout << "\n⚠️ 잘못된 입력 형태입니다. 숫자를 입력하세요.\n[아무 숫자나 입력 후 엔터]";
                        int d; std::cin >> d; std::cin.clear(); std::cin.ignore();
                        continue;
                    }

                    if (shopChoice == 99) {
                        inShop = false;
                    } 
                    else if (shopChoice >= 0 && shopChoice < shop.getSellItemCount()) {
                        std::cout << "판매할 수량을 입력하세요: ";
                        int qty; std::cin >> qty;
                        if (std::cin.fail() || qty <= 0) {
                            std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                            std::cout << "❌ 잘못된 수량입니다.\n[엔터키 누르기]" << std::flush; getch();
                            continue;
                        }
                        shop.sellToPierre(player, shopChoice, qty);
                        std::cout << "\n[아무 숫자나 입력 후 엔터]"; int dummy; std::cin >> dummy;
                    }
                }
                systemMessage = "물건 판매를 마쳤습니다.";
            }

            else if (key == '5') {
                dayActive = false;
            }

            else if (key == 'q' || key == 'Q') {
                std::cout << "\n정말로 게임을 종료하시겠습니까? (Y/N): " << std::flush;
                char ans = getch();
                if (ans == 'y' || ans == 'Y') isRunning = false;
                else systemMessage = "게임을 계속 진행합니다.";
            }
        }

        if (isRunning) {
            for (int y = 0; y < 12; ++y) {
                for (int x = 0; x < 12; ++x) {
                    Tile& t = map.getTile(x, y);
                    if (t.cropPtr) t.cropPtr->grow();
                    if (t.animalPtr) t.animalPtr->grow(); // 🌟 여기서 매일 밤 `hasProducedToday = false`가 호출됨
                }
            }
            player.sleep();
            currentDay++;
        }
    }

    std::cout << "\n🎉 게임이 종료되었습니다. 최종 골드: " << player.getGold() << "G\n";
    return 0;
}
**Unreal Team Collaboration Guide**


📌 프로젝트 기본 원칙

1. 맵(.umap)은 자동 병합되지 않는다

2. 같은 파일을 동시에 수정하지 않는다

3. 작업 내용은 최소 하루 1회 Push

4. 공용 설정 파일 수정 시 반드시 공유



🌿 브랜치 전략

main        → 항상 실행 가능한 최종 버전
develop     → 통합 작업 브랜치
feature/이름 → 개인 작업 브랜치 (이름부분에 자기 원하는대로 브랜치이름)



🛠 작업 시작 절차

git checkout develop
git pull
git checkout feature/이름



💾 작업 종료 절차

git add .
git commit -m "작업 내용"
git push

→ GitHub에서 Pull Request 생성



 🗺 *맵 작업 규칙 (중요)*
 1. 수정 전 반드시 Lock
git lfs lock Content/Maps/파일명.umap

작업 종료 후:

git lfs unlock Content/Maps/파일명.umap
Lock 없이 맵 수정 금지
// 지원씨가 올려준 git lfs 참고

 2. Save All 누르기 전 확인

저장 전 반드시:

다른 레벨이 dirty(*) 상태인지 확인

담당 Sublevel만 저장

무조건 Save All 금지



⚙️ 공용 파일 수정 규칙

다음 파일 수정 시 반드시 공유:

DefaultEngine.ini

MGSProjectCharacter

GameMode

PlayerController

gameinstance ...(게임프레임워크 파일)



📝 Pull Request 작성 규칙

PR 설명에 반드시 포함:

무엇을 수정했는지

맵 변경 여부



🚫 금지 사항

develop에서 직접 작업

main에 직접 push

Lock 없이 맵 수정

다른 사람 Sublevel 수정
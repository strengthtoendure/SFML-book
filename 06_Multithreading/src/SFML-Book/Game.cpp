#include <SFML-Book/Game.hpp>
#include <SFML-Book/Configuration.hpp>
#include <SFML-Book/Piece.hpp>

#include <sstream>

namespace book
{
    Game::Game(int X, int Y,int word_x,int word_y) : ActionTarget(Configuration::player_inputs), _window(sf::VideoMode(X,Y),"06_Multithreading"),_current_piece(nullptr), _world(word_x,word_y), _mainMenu(_window),_configurationMenu(_window),_pauseMenu(_window),_status(Status::StatusMainMenu), _physics_thread(&Game::update_physics,this),_is_running(true)
    {
        bind(Configuration::PlayerInputs::HardDrop,[this](const sf::Event&){        
             sf::Lock lock(_mutex);
             _current_piece = _world.newPiece();
             timeSinceLastFall = sf::Time::Zero;
        });

        bind(Configuration::PlayerInputs::TurnLeft,[this](const sf::Event&){
             _rotate_direction-=1;
        });
        bind(Configuration::PlayerInputs::TurnRight,[this](const sf::Event&){        
             _rotate_direction+=1;
        });

        bind(Configuration::PlayerInputs::MoveLeft,[this](const sf::Event&){        
             _move_direction-=1;
        });

        bind(Configuration::PlayerInputs::MoveRight,[this](const sf::Event&){        
             _move_direction+=1;
        });


        _stats.setPosition(BOOK_BOX_SIZE*(word_x+3),BOOK_BOX_SIZE);

        initGui();

    }

    void Game::run(int minimum_frame_per_seconds, int physics_frame_per_seconds)
    {
        sf::Clock clock;
        const sf::Time timePerFrame = sf::seconds(1.f/minimum_frame_per_seconds);
        _physics_frame_per_seconds = physics_frame_per_seconds;

        _physics_thread.launch();

        while (_window.isOpen())
        {
            sf::Time time = clock.restart();

            processEvents();

            if(_status == StatusGame and not _stats.isGameOver())
            {
                update(time,timePerFrame);
            }

            render();
        }
        _is_running = false;
        _physics_thread.wait();
    }
    void Game::update(const sf::Time& deltaTime,const sf::Time& timePerFrame)
    {
        static sf::Time timeSinceLastUpdate = sf::Time::Zero;

        timeSinceLastUpdate+=deltaTime;
        timeSinceLastFall+=deltaTime;

        if(timeSinceLastUpdate > timePerFrame)
        {
            sf::Lock lock(_mutex);
            if(_current_piece != nullptr)
            {
                _current_piece->rotate(_rotate_direction*3000);
                _current_piece->moveX(_move_direction*5000);

                bool new_piece;
                {
                    int old_level =_stats.getLevel();
                    _stats.addLines(_world.clearLines(new_piece,*_current_piece));
                    if(_stats.getLevel() != old_level)
                        _world.add(Configuration::Sounds::LevelUp);
                }

                if(new_piece or timeSinceLastFall.asSeconds() > std::max(1.0,10-_stats.getLevel()*0.2))
                {
                    _current_piece = _world.newPiece();
                    timeSinceLastFall = sf::Time::Zero;
                }
            }
            _world.update(timePerFrame);
            _stats.setGameOver(_world.isGameOver());
            timeSinceLastUpdate = sf::Time::Zero;
        }
        _rotate_direction=0;
        _move_direction=0;
    }

    void Game::update_physics()
    {
        sf::Clock clock;
        const sf::Time timePerFrame = sf::seconds(1.f/_physics_frame_per_seconds);
        sf::Time timeSinceLastUpdate = sf::Time::Zero;

        while (_is_running)
        {
            sf::Lock lock(_mutex);

            timeSinceLastUpdate+= clock.restart();

            _world.updateGravity(_stats.getLevel());

            while (timeSinceLastUpdate > timePerFrame)
            {

                if(_status == StatusGame and not _stats.isGameOver())
                    _world.update_physics(timePerFrame);

                timeSinceLastUpdate -= timePerFrame;
            }
        }
    }

    void Game::initGui()
    {
        //_mainMenu
        {
            book::gui::VLayout* layout = new book::gui::VLayout;
            layout->setSpace(25);

            book::gui::TextButton* newGame = new book::gui::TextButton("New Game");
            newGame->on_click = [this](const sf::Event&, book::gui::Button& button){
                initGame();
                _status = Status::StatusGame;
            };
            layout->add(newGame);

            book::gui::TextButton* configuration = new book::gui::TextButton("Configuration");
            configuration->on_click = [this](const sf::Event&, book::gui::Button& button){
                _status = Status::StatusConfiguration;
            };
            layout->add(configuration);

            book::gui::TextButton* exit = new book::gui::TextButton("Exit");
            exit->on_click = [this](const sf::Event&, book::gui::Button& button){
                _window.close();
            };
            layout->add(exit);

            _mainMenu.setLayout(layout);

            _mainMenu.bind(Configuration::GuiInputs::Escape,[this](const sf::Event& event){
                               this->_window.close();
                           });
        }
        //_pauseMenu
        {
            book::gui::VLayout* layout = new book::gui::VLayout;
            layout->setSpace(50);

            book::gui::Label* pause = new book::gui::Label("Pause");
            pause->setCharacterSize(70);
            layout->add(pause);

            book::gui::TextButton* exit = new book::gui::TextButton("Exit");
            exit->on_click = [this](const sf::Event&, book::gui::Button& button){
                _status = StatusMainMenu;
            };
            layout->add(exit);

            _pauseMenu.setLayout(layout);

            _pauseMenu.bind(Configuration::GuiInputs::Escape,[this](const sf::Event& event){
                _status = StatusGame;
            });
        }
        
        //configuration
        {
            auto title = sfg::Label::Create("Enter your starting level");
            auto level = sfg::Entry::Create();
            auto error = sfg::Label::Create();
            auto button = sfg::Button::Create( "Ok" );
            button->GetSignal( sfg::Button::OnLeftClick ).Connect(
              [level,error,this](){
                int lvl = 0;
	            std::stringstream sstr(static_cast<std::string>(level->GetText()));
	            sstr >> lvl;
                if(lvl < 1 or lvl > 100)
                    error->SetText("Enter a number from 1 to 100.");
                else
                {
                    error->SetText("");
                    initGame();
                    _stats.setLevel(lvl); 
                    _status = Status::StatusGame;
                }
              }
            );

            auto table = sfg::Table::Create();
            table->SetRowSpacings(10);

            table->Attach(title,sf::Rect<sf::Uint32>(0,0,1,1));
            table->Attach(level,sf::Rect<sf::Uint32>(0,1,1,1));
            table->Attach(button,sf::Rect<sf::Uint32>(0,2,1,1));
            table->Attach(error,sf::Rect<sf::Uint32>(0,3,1,1));

            table->SetAllocation(sf::FloatRect((_window.getSize().x-300)/2,
                                                (_window.getSize().y-200)/2,
                                                300,200));
            _sfg_desktop.Add(table);

            _configurationMenu.bind(Configuration::GuiInputs::Escape,[this](const sf::Event& event){
                _status = StatusMainMenu;
            });
        }
    }

    void Game::initGame()
    {
        sf::Lock lock(_mutex);
        timeSinceLastFall = sf::Time::Zero;

        _stats.reset();
        _world.reset();

        _current_piece = _world.newPiece();
    }

    void Game::processEvents()
    {
        //to store the events
        sf::Event event;
        //events loop
        while(_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)//Close window
                _window.close();
            else if (event.type == sf::Event::KeyPressed and event.key.code == sf::Keyboard::Escape and _status == Status::StatusGame)
            {
                _status = StatusPaused;
            }
            else
            {
                switch(_status)
                {
                    case StatusMainMenu:
                    {
                        _mainMenu.processEvent(event);
                    }break;
                    case StatusGame :
                    {
                        ActionTarget::processEvent(event);
                    }break;
                    case StatusConfiguration :
                    {
                        _configurationMenu.processEvent(event);
                        _sfg_desktop.HandleEvent(event);
                    }break;
                    case StatusPaused :
                    {
                        _pauseMenu.processEvent(event);
                    }break;
                    default : break;
                }
            }
        }

        switch(_status)
        {
            case StatusMainMenu:
            {
                _mainMenu.processEvents();
            }break;
            case StatusGame :
            {
                ActionTarget::processEvents();
            }break;
            case StatusConfiguration :
            {
                _configurationMenu.processEvents();
            }break;
            case StatusPaused :
            {
                _pauseMenu.processEvents();
            }break;
            default : break;
        }
    }

    void Game::render()
    {
        _window.clear();

        switch(_status)
        {
            case StatusMainMenu:
            {
                _window.draw(_mainMenu);
            }break;
            case StatusGame :
            {
                if(not _stats.isGameOver())
                {
                    sf::Lock lock(_mutex);
                    _window.draw(_world);
#ifdef BOOK_DEBUG
                    _world.displayDebug();
#endif
                }
                _window.draw(_stats);

            }break;
            case StatusConfiguration:
            {
                _sfg_desktop.Update(0.0);
                _sfgui.Display(_window);
                _window.draw(_configurationMenu);
            }break;
            case StatusPaused :
            {
                if(not _stats.isGameOver())
                {
                    sf::Lock lock(_mutex);
                    _window.draw(_world);
                }
                _window.draw(_pauseMenu);
            }break;
            default : break;
        }

        _window.display();

    }
}

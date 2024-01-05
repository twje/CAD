#include <SFML/Graphics.hpp>

#include <iostream>

class Application
{
public:
    Application()
        : mWindow(sf::VideoMode(sf::Vector2u(1600, 960), 32), "SFML works!")
    {
        mView = mWindow.getDefaultView();
    }

    void Run()
    {
        bool isMiddleButtonPressed = false;
        sf::Vector2i lastPanPosition;        

        while (mWindow.isOpen())
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);
            float zoomIncrement = 0.0f;

            sf::Event event;
            while (mWindow.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    mWindow.close();
                }

                // Pan
                if (event.type == sf::Event::MouseButtonPressed)
                {
                    if (event.mouseButton.button == sf::Mouse::Button::Middle)
                    {
                        lastPanPosition = mousePosition;
                        isMiddleButtonPressed = true;                        
                    }
                }

                if (event.type == sf::Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == sf::Mouse::Button::Middle)
                    {
                        isMiddleButtonPressed = false;
                    }
                }

                // Zoom
                if (event.type == sf::Event::MouseWheelScrolled)
                {
                    if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::Vertical)
                    {                                   
                        if (event.mouseWheelScroll.delta == 1)
                        {
                            zoomIncrement = mZoomSpeed;
                        }
                        else if (event.mouseWheelScroll.delta == -1)
                        {
                            zoomIncrement = -mZoomSpeed;
                        }   
                    }
                }               
            }

            if (zoomIncrement != 0)
            {
                sf::Vector2f mouseWorldBeforeZoom = mWindow.mapPixelToCoords(mousePosition, mView);
                mView.zoom(1.0f / mZoomFactor);  // Reset zoom factor to 1.0f
                mZoomFactor = Clamp(mZoomFactor + zoomIncrement, mZoomMin, mZoomMax);
                mView.zoom(mZoomFactor);

                sf::Vector2f mouseWorldAfterZoom = mWindow.mapPixelToCoords(mousePosition, mView);
                sf::Vector2f adjustment = mouseWorldBeforeZoom - mouseWorldAfterZoom;
                mView.move(adjustment);
            }

            if (isMiddleButtonPressed)
            {          
                // Convert the last pan position and current mouse position to world coordinates
                sf::Vector2f worldLastPanPosition = mWindow.mapPixelToCoords(lastPanPosition, mView);
                sf::Vector2f worldMousePosition = mWindow.mapPixelToCoords(mousePosition, mView);
                
                // Calculate the pan offset in world coordinates
                sf::Vector2f panOffsetWorld = worldLastPanPosition - worldMousePosition;
                mView.move(panOffsetWorld);

                // Update the last pan position (in screen coordinates)
                lastPanPosition = mousePosition;               
            }

            sf::Vector2f worldMousePosition = mWindow.mapPixelToCoords(mousePosition, mView);

            float nearestGridX = std::round(worldMousePosition.x / mGridSpacing) * mGridSpacing;
            float nearestGridY = std::round(worldMousePosition.y / mGridSpacing) * mGridSpacing;

            mCursor.x = nearestGridX;
            mCursor.y = nearestGridY;

            mWindow.setView(mView);
            mWindow.clear();
            
            sf::Vector2f viewSize = mView.getSize();
            sf::Vector2f topLeftWorld = mWindow.mapPixelToCoords({ 0, 0 }, mView);

            topLeftWorld.x = std::floor(topLeftWorld.x / mGridSpacing) * mGridSpacing;
            topLeftWorld.y = std::floor(topLeftWorld.y / mGridSpacing) * mGridSpacing;

            for (size_t gridX = 0; gridX < viewSize.x + mGridSpacing; gridX += mGridSpacing)
            {
                for (size_t gridY = 0; gridY < viewSize.y + mGridSpacing; gridY += mGridSpacing)
                {
                    DrawPoint({ gridX + topLeftWorld.x, gridY + topLeftWorld.y }, 3.0f, sf::Color::Green);
                }
            }
            
            DrawPoint(mCursor, 3.0f, sf::Color::Red);

            mWindow.display();           
        }
    }

private:
    void DrawPoint(sf::Vector2f position, float radius, sf::Color color)
    {
        sf::CircleShape shape(radius);
        shape.setFillColor(color);
        shape.setPosition({ position.x - radius, position.y - radius });
        mWindow.draw(shape);
    }

    float Clamp(float value, float minValue, float maxValue) 
    {
        return std::max(minValue, std::min(value, maxValue));
    }

    sf::RenderWindow mWindow;    
    sf::View mView;    
    float mZoomFactor = 1.0f;
    float mZoomSpeed = 0.1f;
    float mZoomMin = 0.3f;
    float mZoomMax = 1.7f;
    float mGridSpacing = 70.0f;
    sf::Vector2f mCursor;
};

int main()
{
    Application app;
    app.Run();

    return 0;
}
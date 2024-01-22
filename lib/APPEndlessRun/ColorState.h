/* MIT License
 *
 * Copyright (c) 2023 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Startup state
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Application
 *
 * @{
 */

#ifndef COLOR_STATE_H
#define COLOR_STATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IState.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The system startup state. */
class ColorState : public IState
{
public:
    /** Traffic light color IDs. */
    typedef enum : uint8_t
    {
        COLOR_ID_IDLE = 0,  /* Do nothing. */
        COLOR_ID_RED,       /* Traffic light color Red. */
        COLOR_ID_GREEN,     /* Traffic light color Green. */
        COLOR_ID_YELLOW_GR, /* Traffic light color Yellow transition Green to Red. */
        COLOR_ID_YELLOW_RG  /* Traffic light color Yellow transition Red to Green. */
    } TLCId;

    /**
     * Get state instance.
     *
     * @return State instance.
     */
    static ColorState& getInstance()
    {
        static ColorState instance;

        /* Singleton idiom to force initialization during first usage. */

        return instance;
    }

    /**
     * If the state is entered, this method will called once.
     */
    void entry() final;

    /**
     * Processing the state.
     *
     * @param[in] sm State machine, which is calling this state.
     */
    void process(StateMachine& sm) final;

    /**
     * If the state is left, this method will be called once.
     */
    void exit() final;

    /**
     * Get new traffic light color.
     *
     * @param[in] tlcId The ID of the current traffic light color.
     */
    void execute(TLCId tlcId)
    {
        m_tlcId = tlcId;
    }

protected:
private:
    TLCId m_tlcId; /* Current traffic light color ID. */

    /**
     * Default constructor.
     */
    ColorState() : m_tlcId(COLOR_ID_IDLE)
    {
    }

    /**
     * Default destructor.
     */
    ~ColorState()
    {
    }

    /* Not allowed. */
    ColorState(const ColorState& state);            /**< Copy construction of an instance. */
    ColorState& operator=(const ColorState& state); /**< Assignment of an instance. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* COLOR_STATE_H */
/** @} */

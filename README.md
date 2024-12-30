# Embedded Systems [IS1300]

## 🧑‍💻 Project Description
This repository contains my entire course project at KTH's *Embedded Systems* course.
The project uses the `Nucleo-L476RG` development board and a custom-made Traffic Light
Shield, both of which are provided by the university. The goal of the project was to 
simulate realistic traffic lights of a four-way intersection, managing both vehicles 
and pedestrians.

The project consisted of three tasks in total and each task had multiple requirements. 
Since the goal was to simulate traffic lights, task scheduling and management played 
an important role. The project was graded based on how many tasks were implemented. 
I chose to implement all three tasks in my project.

If you're interested in reading, an in-depth description of the project and development, I've 
included my [**project report**](https://github.com/Hazofinho/IS1300-Embedded-Systems/blob/main/Arvin%20Kunalic%20PRO1-Report.pdf) in the repository the as well. 

---

## 🛜 Installing and Running
The project's core was generated and configured with STMicroelectronic's `STM32CubeIDE`.
If you'd like to import this project in the IDE:

1. **Clone this repository or download the directory 'PRO1_Arvin_Kunalic'**
2. In `STM32CubeIDE:`
    - Navigate to ` File -> Import -> Import Projects from File System or Archive `
    - Select ` Import Sources -> /path/to/the/downloaded/repository-or-project-directory `  
3. Click the `Finish` button
4. You should now be able to view all of the code, along with the configured peripherals.

Unfortunately, flashing the project onto a `Nucleo-L476RG` <small>(or any other STM32)</small> board will not 
work, since the project is designed to make use of the custom-made shield that carries all 
the components. However, it might still be interesting to view the source-code through the
`STM32CubeIDE` to get an idea of how timer, interrupt and GPIO configurations can be made 
using the UI, if you're not familiar with the IDE or STM32 in general.

---

## ⚖️ License
[ArvinKuna]

**You are permitted to:**
- Download and use the files for personal or non-commercial purposes.

**You are NOT permitted to:**    
- Redistribute, share, or publish the files in any form.
- Modify or create derivative works based on these files.
- Use the files for any commercial purpose without explicit permission from the copyright holder.

For any other use, please contact [Arvin Kunalic] at [***github.com/Hazofinho***](https://github.com/Hazofinho).

Copyright [2024] [Arvin Kunalic]. All rights reserved.

---